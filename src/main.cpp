/*
      Project code version/date: Lightning Controller 2.0 / 10-januari-2024

      Project Location: Users/harrywieldraaijer/Documents/PlatformIO/Projects/LigthController2.0/Project/Lightning controller 2.0

      Created by: Harry Wieldraaijer

      Framework version/date: 11.0 - 8-januari-2024

      Short description: This code will only work with  ESSP-32 !
     
  Version history:
    
    Version 0.28 - 19-januari-2024    Fixed and extended the debug issue.(Saved on github)
                                      Improved function naming.
                                      Moved KlikAanKlikUit related code to moduleKlikAanKlikUit.h
                                      Moved DS3231 related code to moduleDS3231.h
                                      Moved commonly used code to moduleCommon.h
                                      Moved HTTP code to moduleHTTPfunctions.h
                                      Moved WIFI code to moduleConnctToWIFI.h
                                      platformio.ini is now multi-platform
                                      Added code for test buttons.
    Version 0.27 - 18-januari-2024    First working version ready for aplha testing (Saved on github)
    Version 0.26 - 15-januari-2024    Set up first running program next step (Saved on github)
    Version 0.25 - 15-januari-2024    Replaced Serial.print with Serial.printf (Saved on github)
    Version 0.24 - 14-januari-2024    Set up first running program next step
    Version 0.23 - 14-januari-2024    Set up first running program next step
    Version 0.22 - 13-januari-2024    Set up first running program
    Version 0.21 - 11-januari-2024    What to do when no /config.json is found
    Version 0.2 - 10-januari-2024     Added and updated HTML on webClient. Store config in /config.json
    Version 0.1 - 8-januari-2024      Initial version

*/

// Include all project modules here - They include their own libaries
#include "moduleCommon.h"
#include "moduleKlikAanKlikUit.h"
#include "moduleHTTPfunctions.h"
#include "moduleDS3231.h"
#include "moduleConnectToWIFI.h"

// Import required libraries
#if defined(ESP8266)
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <AsyncTCP.h>
#endif
#include "LittleFS.h"

#define movementInterruptPin 19

// Define running semaphores
int SEMAPHORE_PREVIOUS_BLOCK_NUMBER = 0;
int SEMAPHORE_CURRENT_BLOCK_NUMBER = 0;
bool SEMAPHORE_MOVEMENT_DETECTED = false;
bool SEMAPHORE_MOVEMENT_ACTIVE = false;
int SEMAPHORE_INTERRUPT_DURATION = 0; // Minutes
unsigned long SEMAPHORE_MOVEMENT_START = 0;
bool SEMAPHORE_TIMER_INTERRUPT = false;

char curLat[] = "52.3780389"; // Location parameters Aadorp (Oost!)
char curLng[] = "6.6234012";

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// This tells the compiler that this function exists It will be defined beneat!
void defineRunningParameters();

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    debug1("An error has occurred while mounting LittleFS");
  }
    debug1("LittleFS mounted successfully");
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
    debug2("The size of the config  file is: %i\n",sizeof(readConfig)); // Used to define the size of currrentConfig
    configFile.close(); 
    readConfig[filesize] = '\0';
    strcpy(currentConfig,readConfig);
    configFile.close();  
  }
  debug2("The current configuration is: %s\n",currentConfig); 
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
    char tstValue[10];
    strncpy(tstValue,(char*)data,10);
    tstValue[10]=0;
    // debug1("The tstValue is: %s\n",tstValue);
    if (strcmp((char*)data, "configuration") == 0) {
      debug1("This client requests for configuration settings\n");
      readConfigFile();
      ws.textAll(currentConfig);
    }  else
    if (strcmp(tstValue, "testButton") == 0) {
        // Code for the testbutton
        debug2("Test button pressed\n");
        debug2("The received value is: %s\n",(char*)data);
        char incomingData[24];
        strcpy(incomingData,(char*)data);
        //int myLen = strlen(incomingData);
        debug2("The length is: %i\n",strlen(incomingData));
        int testLevel=0;
        if (strlen(incomingData) == 11){
          char tmpLevel[3];
          strncpy(tmpLevel,&incomingData[10],1);
          testLevel = atoi(tmpLevel);
          debug1("desired value is: %i\n",testLevel);
          KlikAanKlikUit::setDimmer(testLevel);
        } else { // len = 12
          char tmpLevel[3];
          strncpy(tmpLevel,&incomingData[10],2);
          testLevel = atoi(tmpLevel);
          debug1("desired value is: %i\n",testLevel);
          KlikAanKlikUit::setDimmer(testLevel);
        }
    } else 
      { // we received an update of the configuration settings
      char receivedConfig[768];
      strcpy (receivedConfig, (char*)data);
      debug1("Received JSON string: %s\n",receivedConfig);
      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        debug1("failed to open config file for writing\n");
      }
      if (configFile.print(receivedConfig)){
        debug1("The configuration has successfully been written\n");
        } 
      else {
        debug1("The configuration could not be stored!\n");
        }
        configFile.close();
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
    debug1("deserializeJson() failed with code %s\n",err.c_str());
  } else {

  // Example of retrieving a parameter
  //const int tmpVar = atoi(doc["settings"]["block1"]["lightLevel"]);
  //Serial.printf("Example: Block1 Light Level: %i\n",tmpVar);
 
  // Get sunrise and sunset
  char timeSunrisetimeSunset[24];
  HTTPfunctions::timeSunriseSunset(curLat, curLng, timeSunrisetimeSunset);
  char sunriseChar[6];
  strncpy(sunriseChar,&timeSunrisetimeSunset[0],5);
  int sunriseInteger = DS3231rtc::calcTime(sunriseChar);
  char sunsetChar[6];
  strncpy(sunsetChar,&timeSunrisetimeSunset[6],5);
  int sunsetInteger = DS3231rtc::calcTime(sunsetChar);
  
  // Filling block1

  const bool block1StartMomentFixed = doc["settings"]["block1"]["startMomentFixed"];
  if (block1StartMomentFixed == true) {
    char startTime[5];
    strcpy(startTime,doc["settings"]["block1"]["startTime"]);
    int timeInteger = DS3231rtc::calcTime(startTime);
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
    int timeInteger = DS3231rtc::calcTime(startTime);
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
    int timeInteger = DS3231rtc::calcTime(startTime);
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
  debug1("%s : \n",blockNumber);
  debug1("blockStart: %i\n",block.blockStart);
  debug1("blockEnd: %i\n",block.blockEnd);
  debug1("blockBaseLevel: %i\n",block.blockBaseLevel);
  debug1("blockIntEnabled: %i\n",block.blockIntEnabled);
  debug1("blockIntLevel: %i\n",block.blockIntLevel);
  debug1("blockIntDuration: %i\n\n",block.blockIntDuration);
}

int getCurrentBlockNumber(){
  int currentblockNumber=0;
  int block1Start = settingsBlock1.blockStart;
  int block1End = settingsBlock1.blockEnd;
  int block2Start = settingsBlock2.blockStart;
  int block2End = settingsBlock2.blockEnd;
  int block3Start = settingsBlock3.blockStart;
  int block3End = settingsBlock3.blockEnd;
  DateTime now = DS3231rtc::rtc.now();
  char timeString[6];
  sprintf(timeString, "%02i:%02i",now.hour(),now.minute());
  debug1("The current time is: %s\n",timeString);
  int timeInteger = DS3231rtc::calcTime(timeString);
  debug2("The integer value of the current time is: %i\n",timeInteger);
  if (timeInteger >= block1Start && timeInteger <= block1End){
    currentblockNumber=1;
  } else
  if (timeInteger >= block3Start && timeInteger <= block3End){
    currentblockNumber=3;
  } else {
  currentblockNumber=2;
  }
  debug2("Current block number: %i\n",currentblockNumber);
  return currentblockNumber;
}

void updateSettings(int currentBlockNumber, int selectedIntenity){
  // selectedIntenity 0 = Default, 1 = Interrupt
  int baseLevel;
  int interruptLevel;
  switch (currentBlockNumber) {
    case 1:
        baseLevel = settingsBlock1.blockBaseLevel;
        if (settingsBlock1.blockIntEnabled == 0) {
          interruptLevel=settingsBlock1.blockBaseLevel;
        } else
        if (settingsBlock1.blockIntEnabled == 1) {
          interruptLevel=settingsBlock1.blockIntLevel;
        }
        break;
      case 2:
        baseLevel = settingsBlock2.blockBaseLevel;
        if (settingsBlock2.blockIntEnabled == 0) {
          interruptLevel=settingsBlock2.blockBaseLevel;
        } else
        if (settingsBlock2.blockIntEnabled == 1) {
          interruptLevel=settingsBlock2.blockIntLevel;
        }
        break;
      case 3:
        baseLevel = settingsBlock3.blockBaseLevel;
        if (settingsBlock3.blockIntEnabled == 0) {
          interruptLevel=settingsBlock3.blockBaseLevel;
        } else
        if (settingsBlock3.blockIntEnabled == 1) {
          interruptLevel=settingsBlock3.blockIntLevel;
        }
        break;
  }
  if (selectedIntenity == 0) {
      debug1("Requested level: %i\n",baseLevel);
      KlikAanKlikUit::setDimmer(baseLevel);
  } else {
      if (selectedIntenity == 1) {
        debug1("Requested level: %i\n",interruptLevel);
        KlikAanKlikUit::setDimmer(interruptLevel);
    }
  }
}

int getInterruptDuration(int currentBlockNumber) {
  int currentDuration;
  switch (currentBlockNumber) {
      case 1:
        currentDuration = settingsBlock1.blockIntDuration;
      break;
      case 2:
        currentDuration = settingsBlock2.blockIntDuration;
      break;
      case 3:
        currentDuration = settingsBlock3.blockIntDuration;
      break;
  }
  return currentDuration;
}

// Create a timer for 45 seconds
hw_timer_t *Timer0_Cfg = NULL;
void IRAM_ATTR Timer0_ISR()
{
    SEMAPHORE_TIMER_INTERRUPT = true;
}

void IRAM_ATTR motionDetected() {
   SEMAPHORE_MOVEMENT_DETECTED = true;
}

// ------------------ Initialization section ------------------

void setup(){
  Serial.begin(115200);
  Serial.flush();
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif
  // DebugOutputPort.setDebugOutput(true);
  // ESP.wdtDisable(); // Used to debug, disable wachdog timer.
  debug1("\n");

// ------------------ Initialize LittleFS ------------------
  initLittleFS();

if (!WIFIfunctions::connectToWifi ()){
  debug1("\nCould not connect to WIFI network!\n");
  delay(5000);
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
  ElegantOTA.onStart(common::onOTAStart);
  ElegantOTA.onProgress(common::onOTAProgress);
  ElegantOTA.onEnd(common::onOTAEnd);

  // Start server
  server.begin();

  // ------------------ Project section ------------------
  pinMode(movementInterruptPin, INPUT_PULLUP);
  attachInterrupt(movementInterruptPin, motionDetected, RISING); 

  if (! DS3231rtc::rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (DS3231rtc::rtc.lostPower()) {
    debug1("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    DS3231rtc::rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

DS3231rtc::setRTC();
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
  debug1("\n Setup completed!\n");
}

void loop() {
  ws.cleanupClients();
  ElegantOTA.loop();

  // Start project code here

  // Main loop

    if (SEMAPHORE_MOVEMENT_DETECTED){
      SEMAPHORE_CURRENT_BLOCK_NUMBER = getCurrentBlockNumber();
      //Set counter, update output
      SEMAPHORE_MOVEMENT_START = millis();
      debug2("SEMAPHORE_MOVEMENT_START: %i\n",SEMAPHORE_MOVEMENT_START);
      SEMAPHORE_INTERRUPT_DURATION = getInterruptDuration(SEMAPHORE_CURRENT_BLOCK_NUMBER);
      debug2("SEMAPHORE_INTERRUPT_DURATION: %i\n",(SEMAPHORE_INTERRUPT_DURATION*60*1000));
      updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,1); // 0 = Default, 1 = Interrupt
      SEMAPHORE_MOVEMENT_ACTIVE = true;
      SEMAPHORE_MOVEMENT_DETECTED = false;
    }

    if (SEMAPHORE_MOVEMENT_ACTIVE) {
      // Use the value from the the previous block SEMAPHORE_CURRENT_BLOCK_NUMBER = getCurrentBlockNumber();
      // have we reached the end of this interrupt block?
      if (millis() >= SEMAPHORE_MOVEMENT_START + (SEMAPHORE_INTERRUPT_DURATION*1000*60)){
        updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,0); // 0 = Default, 1 = Interrupt 
        SEMAPHORE_MOVEMENT_ACTIVE = false;
      }
    }
    
    if (SEMAPHORE_TIMER_INTERRUPT) {
      SEMAPHORE_CURRENT_BLOCK_NUMBER = getCurrentBlockNumber();
      debug1("Current block number: %i\n",SEMAPHORE_CURRENT_BLOCK_NUMBER);
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
        if(SEMAPHORE_PREVIOUS_BLOCK_NUMBER==2 && SEMAPHORE_CURRENT_BLOCK_NUMBER==3){
          // On switching from block2 to block3 adjust clock and time sunrise and sunset
          DS3231rtc::setRTC();
          defineRunningParameters();
        }
        debug1("Update the output here!\n");
        if (SEMAPHORE_MOVEMENT_ACTIVE) {
          updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,1); // 0 = Default, 1 = Interrupt
        } else {
          updateSettings(SEMAPHORE_CURRENT_BLOCK_NUMBER,0); // 0 = Default, 1 = Interrupt
        }
        SEMAPHORE_PREVIOUS_BLOCK_NUMBER = SEMAPHORE_CURRENT_BLOCK_NUMBER ;
      }
      // Define here if we need to update the dimmer every cycle
    }
}