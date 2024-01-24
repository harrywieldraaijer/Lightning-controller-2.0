#pragma Once
/*
    Created by: Harry Wieldraaijer

    Short description: This module contains code and functions for HTTP calls
     
    Version history: 
    
      Version 0.1 - 20-januari-2024    Initial version

    Prerequisites: 
      An active WIFI connection
      
*/
#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <HTTPClient.h>
#endif
#include <WiFiClient.h>
#include <ArduinoJSON.h>

namespace HTTPfunctions {
char* getAPIstringFromWebsite(char* locWebLocation, char* returnValue) { 
    debug2(locWebLocation);
    WiFiClient client;
    HTTPClient http;
    debug1("[HTTP] begin...\n");
    if (http.begin(client, locWebLocation)) {  // HTTP
        Serial.printf("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          debug1("[HTTP] GET... code: %i\n",httpCode);
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String tmpReturnValue = http.getString();
            debug2("The reply of the webserver is: %s\n",tmpReturnValue);
            debug2("The size of the buffer schould be: %i\n",(tmpReturnValue.length())+1);
            tmpReturnValue.toCharArray(returnValue, tmpReturnValue.length()+1);   
         }
      } else {
         debug1("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      debug1("We have closed http now \n");

    } else {
       debug1("[HTTP] Unable to connect\n");
    }
    debug2("The contents to be returned is: %s\n",returnValue);
    return returnValue;
}

char* getJSONvarFromURL(char* locURL,char* locParameter,char* returnValue){
  // First get the requested string from the URL
  const int capacity  = 512; //From JSON assistant
  debug2("Return information from URL: %s\n",locURL);
  getAPIstringFromWebsite(locURL,returnValue);
  debug2("Function returns: %s\n",returnValue); 
  // now retrieve te desired key
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,returnValue);
  if  (err) {
    debug1("deserializeJson() failed with code : %s\n",err.c_str());
    }
    const char* tmpReturnValue =  doc[locParameter];
    if (tmpReturnValue != NULL) {
      strcpy(returnValue,tmpReturnValue);
    }
    else{
      debug1("The key %s is not present\n",locParameter);
      strcpy(returnValue,"KEY NOT FOUND!");
    }
    debug2("The returnvalue is: %s\n",returnValue);
    return (returnValue);
  }
  
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
  debug2("The request URL = %s\n",requestURL);
  char receivedReply[768];
  getAPIstringFromWebsite(requestURL,receivedReply);
  debug2("The recieved information is = %s\n",receivedReply);
  const int capacity  = 256; //From JSON assistant
  StaticJsonDocument<capacity> doc;
  DeserializationError err =  deserializeJson(doc,receivedReply);
  if  (err) {
    debug1("deserializeJson() failed with code %s\n",err.c_str());
    strcpy(timeSunrisetimeSunset,"deserializeJson() failed with code ");
    strcat(timeSunrisetimeSunset,err.c_str());
    } else { 
      const char* tmpSunrise =  doc["results"]["sunrise"];
      if (tmpSunrise != NULL) {
        char showTime[9];
        strncpy(showTime,&tmpSunrise[11],5);
        showTime[5] = '\0';   /* null character manually added */
        strcpy(sunrise,showTime);
        debug2("Sunrise is at: %s\n",sunrise);
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
        debug2("Sunset is at: %s\n",sunset);
      }
      else{
        debug1("The key results.sunset is not present");
        strcpy(timeSunrisetimeSunset,"The key results.sunset is not present");
      }
      timeSunrisetimeSunset[0] = '\0';
      strcat(timeSunrisetimeSunset,sunrise);
      strcat(timeSunrisetimeSunset,"S\0"); // S is seperator character
      strcat(timeSunrisetimeSunset,sunset);
      debug1("The sunset string is %s\n",timeSunrisetimeSunset);
    }
  return timeSunrisetimeSunset;
}
  /* Excample: Get a parameter from an website
  char getBack[512];
  char webLocation[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  Serial.printf("Returninformation from URL: %s\n",webLocation);
  HTTPfunctions::getAPIstringFromWebsite(webLocation,getBack);
  Serial.printf("Function returns: %s\n",getBack); 

  // Example: Get a single parameter from an JSON string retrieved fron a website
  char fromURL[] = "http://worldtimeapi.org/api/timezone/Europe/Amsterdam";
  char requestedParameter[] = "datetime";
  char returnValue[512];
  HTTPfunctions::getJSONvarFromURL(fromURL,requestedParameter,returnValue);
  Serial.printf("The contents of the parameter is: %s\n",returnValue);

  // Example: Get sunrise and sunset for location Aadorp (Oost!)
  char curLat[] = "52.3780389";
  char curLng[] = "6.6234012";
  //char timeSunset[24];
  HTTPfunctions::sunsetTime(curLat, curLng, timeSunset);
  Serial.printf("Sunrise and sunset are today at: %s\n",timeSunset);
  */
}