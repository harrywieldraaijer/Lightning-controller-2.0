/*
      Project code version/date: Lightning Controller 2.0 / 10-januari-2024

      Project Location: Projects/

      Created by: Harry Wieldraaijer

      Framework version/date: 11.0 - 8-januari-2024

      Short description:
     
  Version history:
    
    Version 0.26 - 15-januari-2024    Set up first running program next step
    Version 0.25 - 15-januari-2024    Replaced Serial.print with Serial.printf (Saved on github)
    Version 0.24 - 14-januari-2024    Set up first running program next step
    Version 0.23 - 14-januari-2024    Set up first running program next step
    Version 0.22 - 13-januari-2024    Set up first running program
    Version 0.21 - 11-januari-2024    What to do when no /config.json is found
    Version 0.2 - 10-januari-2024     Added and updated HTML on webClient. Store config in /config.json
    Version 0.1 - 8-januari-2024      Initial version

*/

//  Enable or disable debugging 0 is off / 1 is on
#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define Serial.printf(x)
//#define debugln(x)
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

// Define running semaphores
int SEMAPHORE_PREVIOUS_BLOCK_NUMBER = 0;
int SEMAPHORE_CURRENT_BLOCK_NUMBER = 0;
bool SEMAPHORE_MOVEMENT_DETECTED = false;
bool SEMAPHORE_MOVEMENT_ACTIVE = false;
bool SEMAPHORE_TIMER_INTERRUPT = false;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dns;

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// This tells the compiler that this function exists It will bee defined beneat!
void defineRunningParameters();

void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.printf("Entered config mode %s\n",WiFi.softAPIP());
  // if you used auto generated SSID, print it
  Serial.printf("auto generated SSID: %s\n",myWiFiManager->getConfigPortalSSID());
}

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.printf("An error has occurred while mounting LittleFS");
  }
    Serial.printf("LittleFS mounted successfully");
}

char currentConfig[768];
char* readConfigFile(){
File configFile = LittleFS.open("/config.json", "r");
      if (!configFile) {
        Serial.printf("failed to open config file for reading");
      } else {
        size_t filesize = configFile.size(); //the size of the file in bytes     
        char readConfig[filesize+1 ];   // + 1 for '\0' char at the end      
        configFile.read((uint8_t *)readConfig, sizeof(readConfig));  
        //Serial.printf("The size of the config  file is: %i\n",sizeof(readConfig)); // Used to define the size of currrentConfig
        configFile.close(); 
        readConfig[filesize] = '\0';
        strcpy(currentConfig,readConfig);
        configFile.close();  
      }
// Serial.printf("The current configuration is: %s\n",currentConfig); 
return currentConfig;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  /*  This handles the message received from the clients
      this can be a request for the JSON configuration string ("configuration") or 
      or an action on pressing a testbutton or
      an updated JSON configuration string to be stored
  */
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "configuration") == 0) {
      Serial.printf("Need to send the configuration to the client!\n");
      readConfigFile();
      ws.textAll(currentConfig);
    }  else
    if (strcmp((char*)data, "testbutton") == 0) {
      // Code for the testbutton
    } else 
      { // we received an update of the configuration settings
      char receivedConfig[768];
      strcpy (receivedConfig, (char*)data);
      Serial.printf("Received JSON string: %s\n",receivedConfig);
      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        Serial.printf("failed to open config file for writing\n");
      }
      if (configFile.print(receivedConfig)){
        Serial.printf("The configuration has successfully been written\n");
        } 
      else {
        Serial.printf("The configuration could not be stored!\n");
        }
        configFile.close();
      // Call the function to update the current settings
      defineRunningParameters();
      // Update current parameters
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
  Serial.printf("OTA update started!\n");
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
    Serial.printf("OTA update finished successfully!\n");
  } else {
    Serial.printf("There was an error during OTA update!\n");
  }
  // <Add your own code here>
}

char* GetParameterFromInternet(char* locWebLocation, char* returnValue) { 
    // Serial.printf(locWebLocation);
    WiFiClient client;
    HTTPClient http;
    Serial.printf("[HTTP] begin...\n");
    if (http.begin(client, locWebLocation)) {  // HTTP
        Serial.printf("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %i\n",httpCode);
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String tmpReturnValue = http.getString();
            //Serial.printf("The reply of the webserver is: %s\n",tmpReturnValue);
            //Serial.printf("The size of the buffer schould be: %i\n",(tmpReturnValue.length())+1);
            tmpReturnValue.toCharArray(returnValue, tmpReturnValue.length()+1);   
         }
      } else {
         Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      Serial.printf("We have closed http now \n");

    } else {
       Serial.printf("[HTTP] Unable to connect\n");
    }
     //Serial.printf("The contents to be returned is: %s\n",returnValue);
    return returnValue;
}

char* getVarFromJasonVar(char* locURL,char* locParameter,char* returnValue){
  // First get the requested string from the URL
  const int capacity  = 512; //From JSON assistant
  //Serial.printf("Return information from URL: %s\n",locURL);
  GetParameterFromInternet(locURL,returnValue);
  //Serial.printf("Function returns: %s\n",returnValue); 
  // now retrieve te desired key
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,returnValue);
  if  (err) {
    Serial.printf("deserializeJson() failed with code : %s\n",err.c_str());
    }
    const char* tmpReturnValue =  doc[locParameter];
    if (tmpReturnValue != NULL) {
      strcpy(returnValue,tmpReturnValue);
    }
    else{
      Serial.printf("The key %s is not present\n",locParameter);
      strcpy(returnValue,"KEY NOT FOUND!");
    }
    //Serial.printf("The returnvalue is: %s\n",returnValue);
    return (returnValue);
}

  char curLat[] = "52.3780389";
  char curLng[] = "6.6234012";
  char timeSunrisetimeSunset[24];
  char* timeSunriseSunset(char* curLat, char* curLng, char* timeSunrisetimeSunset){
  char sunrise[32];
  char sunset[32];
  char requestURL[80];
  requestURL[0] = '\0';
  strcat(requestURL,"http://api.sunrise-sunset.org/json?lat=\0");
  strcat(requestURL,curLat);
  strcat(requestURL,"&lng=\0");
  strcat(requestURL,curLng);
  strcat(requestURL,"&formatted=0&tzId=Europe/Amsterdam\0");
  //Serial.printf("The request URL = %s\n",requestURL);
  char receivedReply[768];
  GetParameterFromInternet(requestURL,receivedReply);
  //Serial.printf("The recieved information is = %s\n",receivedReply);
  const int capacity  = 256; //From JSON assistant
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,receivedReply);
  if  (err) {
    Serial.printf("deserializeJson() failed with code %s\n",err.c_str());
    strcpy(timeSunrisetimeSunset,"deserializeJson() failed with code ");
    strcat(timeSunrisetimeSunset,err.c_str());
    } else { 
      const char* tmpSunrise =  doc["results"]["sunrise"];
      if (tmpSunrise != NULL) {
        char showTime[9];
        strncpy(showTime,&tmpSunrise[11],5);
        showTime[5] = '\0';   /* null character manually added */
        strcpy(sunrise,showTime);
        //Serial.printf("Sunrise is at: %s\n",sunrise);
      }
      else{
        Serial.printf("The key results.sunrise is not present");
        strcpy(timeSunrisetimeSunset,"The key results.sunrise is not present");
      }
      const char* tmpSunset =  doc["results"]["sunset"];
      if (tmpSunset != NULL) {
        char showTime[9];
        strncpy(showTime,&tmpSunset[11],5);
        showTime[5] = '\0';   /* null character manually added */
        strcpy(sunset,showTime);
        //Serial.printf("Sunset is at: %s\n",sunset);
      }
      else{
        Serial.printf("The key results.sunset is not present");
        strcpy(timeSunrisetimeSunset,"The key results.sunset is not present");
      }
      timeSunrisetimeSunset[0] = '\0';
      strcat(timeSunrisetimeSunset,sunrise);
      strcat(timeSunrisetimeSunset,"S\0"); // S is seperator character
      strcat(timeSunrisetimeSunset,sunset);
      Serial.printf("The sunset string is %s\n",timeSunrisetimeSunset);
    }
  return timeSunrisetimeSunset;
}

RTC_DS3231 rtc; // Create a rtc instance
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int calcTime(char* localTime){
  int curHour = 0;
  char tmpHour[3];
  strncpy(tmpHour,&localTime[0],2);
  tmpHour[2] = '\0';  /* null character manually added */
  int curMinute1 = 0;
  char tmpMinute[3];
  strncpy(tmpMinute,&localTime[3],2);
  tmpMinute[2] = '\0';  /* null character manually added */
  int timeInteger = (60*atoi(tmpHour))+atoi(tmpMinute);
  //Serial.printf("The current timeInteger is: %i\n",timeInteger);
  return timeInteger;
}

struct settingsBlock{
  int blockStart;
  int blockEnd;
  int blockBaseLevel;
  int blockIntEnabled;
  int blockIntLevel;
  int blockIntDuration;
};

settingsBlock settingsBlock1,settingsBlock2,settingsBlock3;

void defineRunningParameters() {
  // Read configutation
  readConfigFile();
  // currentConfig contains the JSON representation of the current configuration
  const int capacity = 1024; //From JSON assistant
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,currentConfig);
  if  (err) {
    Serial.printf("deserializeJson() failed with code %s\n",err.c_str());
  } else {

  // Example of retrieving a parameter
  //const int tmpVar = atoi(doc["settings"]["block1"]["lightLevel"]);
  //Serial.printf("Example: Block1 Light Level: %i\n",tmpVar);
 
  // Get sunrise and sunset
  timeSunriseSunset(curLat, curLng, timeSunrisetimeSunset);
  char sunriseChar[6];
  strncpy(sunriseChar,&timeSunrisetimeSunset[0],5);
  int sunriseInteger = calcTime(sunriseChar);
  char sunsetChar[6];
  strncpy(sunsetChar,&timeSunrisetimeSunset[6],5);
  int sunsetInteger = calcTime(sunsetChar);
  
  // Filling block1

  const bool block1StartMomentFixed = doc["settings"]["block1"]["startMomentFixed"];
  if (block1StartMomentFixed == true) {
    char startTime[5];
    strcpy(startTime,doc["settings"]["block1"]["startTime"]);
    int timeInteger = calcTime(startTime);
    settingsBlock1.blockStart = timeInteger;
    settingsBlock3.blockEnd=timeInteger - 1; // Yes, block3
  } else {
    settingsBlock1.blockStart = sunsetInteger + atoi(doc["settings"]["block1"]["startShift"]);
    int debugVar = sunsetInteger + atoi(doc["settings"]["block1"]["startShift"]);
  }
  settingsBlock3.blockEnd=settingsBlock1.blockStart-1; //  Yes, block 3
  settingsBlock1.blockBaseLevel=atoi(doc["settings"]["block1"]["lightLevel"]);
  const bool block1InterruptEnabled = doc["settings"]["block1"]["interruptSettings"]["interruptEnabled"];
  if (block1InterruptEnabled == true) {
    settingsBlock1.blockIntEnabled=1;
  } else {
    settingsBlock1.blockIntEnabled=0;
  }
  settingsBlock1.blockIntLevel=atoi(doc["settings"]["block1"]["interruptSettings"]["interruptLightLevel"]);
  settingsBlock1.blockIntDuration=atoi(doc["settings"]["block1"]["interruptSettings"]["interruptDuration"]);
  
  // Filling block 2

  // const bool block2StartMomentFixed = doc["settings"]["block2"]["startMomentFixed"]; Left in for compatibility
  const bool block2StartMomentFixed =  true ; // Fixed
  if (block2StartMomentFixed == true) {
    char startTime[5];
    strcpy(startTime,doc["settings"]["block2"]["startTime"]);
    int timeInteger = calcTime(startTime);
    settingsBlock2.blockStart = timeInteger;
    settingsBlock1.blockEnd=timeInteger - 1; // Yes, block1
  } else {
    settingsBlock2.blockStart = sunriseInteger + atoi(doc["settings"]["block2"]["startShift"]);
    int debugVar = sunriseInteger + atoi(doc["settings"]["block2"]["startShift"]);
     // We must set settingsBlock2.blockEnd=timeInteger - 1; // Yes, block1 - Not urgent because it is fixed
  }
  settingsBlock2.blockBaseLevel=atoi(doc["settings"]["block2"]["lightLevel"]);
  const bool block2InterruptEnabled = doc["settings"]["block2"]["interruptSettings"]["interruptEnabled"];
  if (block2InterruptEnabled == true) {
    settingsBlock2.blockIntEnabled=1;
  } else {
    settingsBlock2.blockIntEnabled=0;
  }
  settingsBlock2.blockIntLevel=atoi(doc["settings"]["block2"]["interruptSettings"]["interruptLightLevel"]);
  settingsBlock2.blockIntDuration=atoi(doc["settings"]["block2"]["interruptSettings"]["interruptDuration"]);
  
  // Filling block 3

  const bool block3StartMomentFixed = doc["settings"]["block3"]["startMomentFixed"];

  if (block3StartMomentFixed == true) {
    char startTime[5];
    strcpy(startTime,doc["settings"]["block3"]["startTime"]);
    int timeInteger = calcTime(startTime);
    settingsBlock3.blockStart = timeInteger;
    settingsBlock2.blockEnd=timeInteger - 1; // Yes, block2
  } else {
    settingsBlock3.blockStart = sunriseInteger + atoi(doc["settings"]["block3"]["startShift"]);
    settingsBlock2.blockEnd = sunriseInteger -1 + atoi(doc["settings"]["block3"]["startShift"]); // Yes, block2
    int debugVar = sunriseInteger + atoi(doc["settings"]["block3"]["startShift"]);
  }
  settingsBlock3.blockBaseLevel=atoi(doc["settings"]["block3"]["lightLevel"]);
  //const bool block3InterruptEnabled = doc["settings"]["block3"]["interruptSettings"]["interruptEnabled"]; Left in for compatibility
  const bool block3InterruptEnabled = false ; // Fixed
  if (block3InterruptEnabled == true) {
    settingsBlock3.blockIntEnabled=1;
  } else {
    settingsBlock3.blockIntEnabled=0;
  }
  settingsBlock3.blockIntLevel=atoi(doc["settings"]["block3"]["interruptSettings"]["interruptLightLevel"]);
  settingsBlock3.blockIntDuration=atoi(doc["settings"]["block3"]["interruptSettings"]["interruptDuration"]);
  }
} 

void displayBlockInfo(const char* blockNumber, const settingsBlock& block) {
  Serial.printf("%s : \n",blockNumber);
  Serial.printf("blockStart: %i\n",block.blockStart);
  Serial.printf("blockEnd: %i\n",block.blockEnd);
  Serial.printf("blockBaseLevel: %i\n",block.blockBaseLevel);
  Serial.printf("blockIntEnabled: %i\n",block.blockIntEnabled);
  Serial.printf("blockIntLevel: %i\n",block.blockIntLevel);
  Serial.printf("blockIntDuration: %i\n\n",block.blockIntDuration);
}

int getCurrentBlockNumber(){
  int currentblockNumber=0;
  int block1Start = settingsBlock1.blockStart;
  int block1End = settingsBlock1.blockEnd;
  int block2Start = settingsBlock2.blockStart;
  int block2End = settingsBlock2.blockEnd;
  int block3Start = settingsBlock3.blockStart;
  int block3End = settingsBlock3.blockEnd;
  DateTime now = rtc.now();
  char timeString[6];
  sprintf(timeString, "%02i:%02i",now.hour(),now.minute());
  Serial.printf("The current time is: %s\n",timeString);
  int timeInteger = calcTime(timeString);
  Serial.printf("The integer value of the current time is: %i\n",timeInteger);
  if (timeInteger >= block1Start && timeInteger <= block1End){
    currentblockNumber=1;
  } else
  if (timeInteger >= block3Start && timeInteger <= block3End){
    currentblockNumber=3;
  } else {
  currentblockNumber=2;
  }
  // Serial.printf("Current block number: %i\n",currentblockNumber);
  return currentblockNumber;
}

// Create a timer for 45 seconds
hw_timer_t *Timer0_Cfg = NULL;
void IRAM_ATTR Timer0_ISR()
{
    SEMAPHORE_TIMER_INTERRUPT = true;
}

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
  Serial.printf("\n");

// ------------------ Initialize LittleFS ------------------
  initLittleFS();

// ------------------ Connect to WIFI network ------------------
  Serial.printf("\n\nConnecting to WIFI network\n\n");

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
    here  "AutoConnectAP" and goes into a blocking loop awaiting configuration
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
  Serial.printf("\nCould not connect to WIFI network!\n");
  delay(5000);
 // ESP.reset(); // Reset and try again
 ESP.restart(); // Reset and try again
}

  // Create a websocket
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

  // Start server
  server.begin();

  /* Excample: Get a parameter from an website
  char getBack[512];
  char webLocation[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  Serial.printf("Returninformation from URL: %s\n",webLocation);
  GetParameterFromInternet(webLocation,getBack);
  Serial.printf("Function returns: %s\n",getBack); 

  // Example: Get a single parameter from an JSON string retrieved fron a website
  char fromURL[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  char requestedParameter[] = "datetime";
  char returnValue[512];
  getVarFromJasonVar(fromURL,requestedParameter,returnValue);
  Serial.printf("The contents of the parameter is: %s\n",returnValue);

  // Example: Get sunrise and sunset for location Aadorp (Oost!)
  char curLat[] = "52.3780389";
  char curLng[] = "6.6234012";
  //char timeSunset[24];
  sunsetTime(curLat, curLng, timeSunset);
  Serial.printf("Sunrise and sunset are today at: %s\n",timeSunset);
  */

  // ------------------ Project section ------------------

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.printf("RTC lost power, let's set the time!");
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

defineRunningParameters();

// Print out the data for each person using the displayBlockInfo function
  displayBlockInfo("settingsBlock 1", settingsBlock1);
  displayBlockInfo("settingsBlock 2", settingsBlock2);
  displayBlockInfo("settingsBlock 3", settingsBlock3);

// Start an interrupt timer (Int. each 55 seeconds)
  Timer0_Cfg = timerBegin(0, 8000, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 550000, true);
  timerAlarmEnable(Timer0_Cfg);
}

void loop() {
  ws.cleanupClients();
  ElegantOTA.loop();

  // Start project code here

  // Main loop
    
    if (SEMAPHORE_MOVEMENT_DETECTED){
      //Set counter, update output
      //updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,1); // 0 = Default, 1 = Interrupt
      SEMAPHORE_MOVEMENT_ACTIVE = true;
      SEMAPHORE_MOVEMENT_DETECTED = false;
    }

    if (SEMAPHORE_MOVEMENT_ACTIVE) {
      // have we reached the end of this interrupt block?
      // for the interrupt duration we use the settings of the moment
      // the interrupt happens
      // but at the end we use the settings of the current situation
      // if we reached the end then
      //updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,0); // 0 = Default, 1 = Interrupt // return to the default settings
    }
    
    if (SEMAPHORE_TIMER_INTERRUPT) {
      SEMAPHORE_CURRENT_BLOCK_NUMBER = getCurrentBlockNumber();
      Serial.printf("Current block number: %i\n",SEMAPHORE_CURRENT_BLOCK_NUMBER);
      switch (SEMAPHORE_CURRENT_BLOCK_NUMBER) {
        case 1:
          displayBlockInfo("Active settings", settingsBlock1);
          break;
        case 2:
          displayBlockInfo("Active settings", settingsBlock2);
          break;
        case 3:
          displayBlockInfo("Active settings", settingsBlock3);
          break;
      }
      SEMAPHORE_TIMER_INTERRUPT = false;
      if (SEMAPHORE_PREVIOUS_BLOCK_NUMBER != SEMAPHORE_CURRENT_BLOCK_NUMBER){
        Serial.printf("Update the output here!\n");
        //updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,1); // 0 = Default, 1 = Interrupt
        SEMAPHORE_PREVIOUS_BLOCK_NUMBER = SEMAPHORE_CURRENT_BLOCK_NUMBER ;
      }
    }
}