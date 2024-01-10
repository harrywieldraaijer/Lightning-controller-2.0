/*
      Project code version/date: Lightning Controller 2.0 / 10-januari-2024

      Project Location: Projects/

      Created by: Harry Wieldraaijer

      Framework version/date: 11.0 - 8-januari-2024

      Short description:
     
  Version history:
    
    Version 0.2 - 10-januari-2024     Added and updated HTML on webClient. Store config in /config.json
    Version 0.1 - 8-januari-2024      Initial version
  
  
*/

//  Enable or disable debugging 0 is off / 1 is on
#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// Import required libraries
#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <HTTPClient.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include "LittleFS.h"
#include <ArduinoJSON.h>
#include <ElegantOTA.h>
#include <WiFiClient.h>

// Import project libraries here

#include <SPI.h>  // required for RTClib.h
#include "RTClib.h"
#define TIME_24_HOUR

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dns;

// unsigned long ota_progress_millis = 0;

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Set number of outputs - Not needed for the project
#define NUM_OUTPUTS  4

// Assign each GPIO to an output - Not needed for the project
int outputGPIOs[NUM_OUTPUTS] = {2, 4, 12, 14};

void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  debugln("Entered config mode");
  debugln(WiFi.softAPIP());
  // if you used auto generated SSID, print it
  debugln(myWiFiManager->getConfigPortalSSID());
}

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    debugln("An error has occurred while mounting LittleFS");
  }
    debugln("LittleFS mounted successfully");
}

/*
String getOutputStates(){
  // This builds the JSON string to be transmitted - Needs to be redefined
  const int capacity = 256;  // from ArduinoJson Assistant
  debug("The capacity of the JSON string is: ");
  debugln(capacity);
  StaticJsonDocument<capacity> myArray;
  for (int i =0; i<NUM_OUTPUTS; i++){
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  String jsonString;
  serializeJson(myArray,jsonString);
  debug("The size of the JSON string is: ");
  debugln(jsonString.length());
  debug("The content of the JSON string is: ");
  debugln(jsonString);
  return jsonString;
}

void notifyClients(String state) {
  // This sends the JSON message
  ws.textAll(state);
}
*/

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  // This handles the messages received from the clients
  // this can be a request for the JSON configuration string or an updated JSON configuration string
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "configuration") == 0) {
      debugln("Need to send the configuration to the client!");
      File configFile = LittleFS.open("/config.json", "r");
      if (!configFile) {
        Serial.println("failed to open config file for reading");
      }
      size_t filesize = configFile.size(); //the size of the file in bytes     
      char readConfig[filesize+1 ];   // + 1 for '\0' char at the end      
      configFile.read((uint8_t *)readConfig, sizeof(readConfig));  
      configFile.close(); 
      readConfig[filesize] = '\0';
      // debug(readConfig); 
      ws.textAll(readConfig);
      configFile.close();  
    }
  else{
      char receivedConfig[768];
      strcpy (receivedConfig, (char*)data);
      // debug("Received JSON string: ");
      // debugln(receivedConfig);
      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        debugln("failed to open config file for writing");
      }
      if (configFile.print(receivedConfig)){
        debugln("The configuration has successfully been written");
        } 
      else {
        debugln("The configuration could not be stored!");
        }
        configFile.close();
      // Call the function to update the current settings
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  // This function decides what to do with actions received from the client
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  // Adds an event to the WebSocket handler
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

char* GetParameterFromInternet(char* locWebLocation, char* returnValue) { 
    // debugln(locWebLocation);
    WiFiClient client;
    HTTPClient http;
    // debug("[HTTP] begin...\n");
    if (http.begin(client, locWebLocation)) {  // HTTP
        // debug("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          // debug("[HTTP] GET... code: ");
          // debugln(httpCode);
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String tmpReturnValue = http.getString();
            /*
            debug("The reply of the webserver is: ");
            debugln(tmpReturnValue);
            debug("The size of the buffer schould be: ");
            debugln((tmpReturnValue.length())+1);
            */
            tmpReturnValue.toCharArray(returnValue, tmpReturnValue.length()+1);   
         }
      } else {
         Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      // debug("We have closed http now ");

    } else {
       debugln("[HTTP] Unable to connect");
    }
     // debug("The contents to be returned is: ");
     // debugln(returnValue);
    return returnValue;
}

char* getVarFromJasonVar(char* locURL,char* locParameter,char* returnValue){
  // First get the requested string from the  URL
  const int capacity  = 512; //From JSON assistant
  // debug("Return information from URL: ");
  // debugln(locURL);
  GetParameterFromInternet(locURL,returnValue);
  // debug("Function returns: "); 
  // debugln(returnValue); 
  // now retrieve te desired key
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,returnValue);
  if  (err) {
    debug(F("deserializeJson() failed with code "));
    debugln(err.c_str());
    }
    const char* tmpReturnValue =  doc[locParameter];
    if (tmpReturnValue != NULL) {
      strcpy(returnValue,tmpReturnValue);
    }
    else{
      debug(F("The key "));
      debug(locParameter);
      debugln(F(" is not present"));
      strcpy(returnValue,"KEY NOT FOUND!");
    }
    // debugln(returnValue);
    return (returnValue);
}

char* sunsetTime(char* curLat, char* curLng, char* timeSunrisetimeSunset){
  char sunrise[32];
  char sunset[32];
  char requestURL[80];
  requestURL[0] = '\0';
  strcat(requestURL,"http://api.sunrise-sunset.org/json?lat=\0");
  strcat(requestURL,curLat);
  strcat(requestURL,"&lng=\0");
  strcat(requestURL,curLng);
  strcat(requestURL,"&formatted=0&tzId=Europe/Amsterdam\0");
  // debug("The request URL = ");
  // debugln(requestURL);
  char receivedReply[768];
  GetParameterFromInternet(requestURL,receivedReply);
  // debug("The recieved information is = ");
  // debugln(receivedReply);
  const int capacity  = 256; //From JSON assistant
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,receivedReply);
  if  (err) {
    debug(F("deserializeJson() failed with code "));
    debugln(err.c_str());
    strcpy(timeSunrisetimeSunset,"deserializeJson() failed with code ");
    strcat(timeSunrisetimeSunset,err.c_str());
    }
    const char* tmpSunrise =  doc["results"]["sunrise"];
    if (tmpSunrise != NULL) {
      char showTime[9];
      strncpy(showTime,&tmpSunrise[11],5);
      showTime[5] = '\0';   /* null character manually added */
      strcpy(sunrise,showTime);
      // debug("Sunrise is at: ");
      // debugln(sunrise);
    }
    else{
      debug(F("The key results.sunrise is not present"));
      strcpy(timeSunrisetimeSunset,"The key results.sunrise is not present");
    }
    const char* tmpSunset =  doc["results"]["sunset"];
    if (tmpSunset != NULL) {
      char showTime[9];
      strncpy(showTime,&tmpSunset[11],5);
      showTime[5] = '\0';   /* null character manually added */
      strcpy(sunset,showTime);
      // debug("Sunset is at: ");
      // debugln(sunset);
    }
    else{
      debug(F("The key results.sunset is not present"));
      strcpy(timeSunrisetimeSunset,"The key results.sunset is not present");
    }
    timeSunrisetimeSunset[0] = '\0';
    strcat(timeSunrisetimeSunset,sunrise);
    strcat(timeSunrisetimeSunset,"S\0"); // S is seperator character
    strcat(timeSunrisetimeSunset,sunset);
    // debugln(timeSunrisetimeSunset);
  return timeSunrisetimeSunset;
}

// ------------------ Project section ------------------
RTC_DS3231 rtc; // Create a rtc instance
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// ------------------ Initialization section ------------------
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200); // For debugging
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif
  // DebugOutputPort.setDebugOutput(true);
  // ESP.wdtDisable(); // Used to debug, disable wachdog timer.
  // Iets van flush doen......
  debug("\n");

// ------------------ Initialize LittleFS ------------------
  initLittleFS();

// ------------------ Connect to WIFI network ------------------
  debugln();
  debugln();
  debug("Connecting to WIFI network");
  debugln();

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server,&dns);

  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //wifiManager.setAPCallback(configModeCallback);

   /*
    set custom ip for portal
    wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    fetches ssid and pass from eeprom and tries to connect
    if it does not connect it starts an access point with the specified name
    here  "AutoConnectAP"
    and goes into a blocking loop awaiting configuration

    or use this for auto generated name ESP + ChipID
    wifiManager.autoConnect();
    */

  wifiManager.autoConnect("AutoConnectAP");
  if (wifiManager.autoConnect()) {;


  debugln();
  debugln("WiFi connected");
  debugln();
  debug("SSID : ");
  debugln(WiFi.SSID());
  debug("MAC address: ");
  debugln(WiFi.macAddress());
  debug("IP address: ");
  debugln(WiFi.localIP());
  debug("WiFi Signal strength: ");
  debug(WiFi.RSSI());
  debug(" dB");
  debugln("");
}
else {
  debugln("");
  debugln("Could not connect to WIFI network!");
  debugln("");
  delay(5000);
 // ESP.reset(); // Reset and try again
 ESP.restart(); // Reset and try again
}

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html",false);
  });

  server.serveStatic("/", LittleFS, "/");

  // Start ElegantOTA
  ElegantOTA.begin(&server); 

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  // Set GPIOs as outputs Must be removed
  for (int i =0; i<NUM_OUTPUTS; i++){
    pinMode(outputGPIOs[i], OUTPUT);
  }

  // Start server
  server.begin();

  // Excample: Get a parameter from an website
  char getBack[512];
  char webLocation[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  // debug("Returninformation from URL: ");
  // debugln(webLocation);
  GetParameterFromInternet(webLocation,getBack);
  Serial.print("Function returns: "); 
  Serial.println(getBack); 

  // Example: Get a single parameter from an JSON string retrieved fron a website
  char fromURL[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  char requestedParameter[] = "datetime";
  char returnValue[512];
  getVarFromJasonVar(fromURL,requestedParameter,returnValue);
  Serial.print("The contents of the parameter is: ");
  Serial.println(returnValue);

  // Example: Get sunrise and sunset for location Aadorp (Oost!)
  char curLat[] = "52.3780389";
  char curLng[] = "6.6234012";
  char timeSunset[24];
  sunsetTime(curLat, curLng, timeSunset);
  Serial.print("Sunrise and sunset are today at: ");
  Serial.println(timeSunset);
  
  // ------------------ Project section ------------------

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

    // When time needs to be re-set on a previously configured device, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

}

// Get the current configuration from LittleFS

// If no current configuration exists create default configuration and store it in LittleFS and continue

void loop() {
  ws.cleanupClients();
  ElegantOTA.loop();

  // Start project code here

    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    Serial.println();

    delay(10000);
}