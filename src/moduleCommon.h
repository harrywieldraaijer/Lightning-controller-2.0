#pragma Once
/*
    Created by: Harry Wieldraaijer


    Short description: This module contains common used code and functions
     
    Version history: 
    
      Version 0.1 - 20-januari-2024    Initial version

    Prerequisites: 
      None
      
*/

#include <Arduino.h>
#include <ElegantOTA.h>

//  Enable or disable debugging / 0 is off / 1 is on / 2 is 1 and 2
#define DEBUG 1
#if DEBUG == 1
  #define debug1(f, ...) Serial.printf(f, ## __VA_ARGS__)
  #define debug2(f, ...) (void)0
#elif DEBUG == 2
  #define debug1(f, ...) Serial.printf(f, ## __VA_ARGS__)
  #define debug2(f, ...) Serial.printf(f, ## __VA_ARGS__)
#else
  #define debug1(f, ...) (void)0
  #define debug2(f, ...) (void)0
#endif

namespace common{
  unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  debug1("OTA update started!\n");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    debug1("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    debug1("OTA update finished successfully!\n");
  } else {
    debug1("There was an error during OTA update!\n");
  }
  // <Add your own code here>
}
}