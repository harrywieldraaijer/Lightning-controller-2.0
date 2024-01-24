#pragma Once
/*
    Created by: Harry Wieldraaijer

    Short description: This module contains DS3231 RTC related code and funtions
     
    Version history: 
    
      Version 0.1 - 20-januari-2024    Initial version

    Prerequisites: 
      DS3231 RTC module
      HTTP active (#include "moduleHTTPfunctions.h")
      
*/
#include <SPI.h>  // required for RTClib.h
#include "RTClib.h"
#define TIME_24_HOUR

char* HTTPfunctions::getJSONvarFromURL(char* fromURL, char* requestedParameter, char* returnValue); // Tell the compiler the function will be defined

namespace DS3231rtc {
    RTC_DS3231 rtc; // Create a rtc instance
    char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    int calcTime(char* localTime){
    // Input hh:mm -> Output integer (60*hh+mm)
    int curHour = 0;
    char tmpHour[3];
    strncpy(tmpHour,&localTime[0],2);
    tmpHour[2] = '\0';  /* null character manually added */
    int curMinute1 = 0;
    char tmpMinute[3];
    strncpy(tmpMinute,&localTime[3],2);
    tmpMinute[2] = '\0';  /* null character manually added */
    int timeInteger = (60*atoi(tmpHour))+atoi(tmpMinute);
    debug2("The current timeInteger is: %i\n",timeInteger);
    return timeInteger;
  }

  void setRTC(){
    char fromURL[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
    char requestedParameter[] = "datetime";
    char returnValue[512];
    HTTPfunctions::getJSONvarFromURL(fromURL,requestedParameter,returnValue);
    debug1("The current date and time is: %s\n",returnValue);
    rtc.adjust(DateTime (returnValue));
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
  }

}