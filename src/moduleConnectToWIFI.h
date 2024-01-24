#pragma Once
/*
    Created by: Harry Wieldraaijer


    Short description: This module contains code and functions to connectto a WIFI network
     
    Version history: 
    
      Version 0.1 - 20-januari-2024    Initial version

    Prerequisites: 
      None
      
*/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <ESPAsyncWiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dns;

namespace WIFIfunctions {
  
      void configModeCallback (AsyncWiFiManager *myWiFiManager) {
        debug1("Entered config mode %s\n",WiFi.softAPIP());
        // if you used auto generated SSID, print it
        debug1("auto generated SSID: %s\n",myWiFiManager->getConfigPortalSSID());
      }
      bool connectToWifi () {
      // ------------------ Connect to WIFI network ------------------
      debug1("\n\nConnecting to WIFI network\n\n");

      //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
      AsyncWiFiManager wifiManager(&server,&dns);
      /*
      reset settings - for testing
        wifiManager.resetSettings();
      
      set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
        wifiManager.setAPCallback(configModeCallback);
      
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
        Serial.print("");
        debug1("WiFi connected\n");
        debug1("\nSSID : %s\n",(WiFi.SSID()));
        debug1("MAC address: ");
        Serial.println(WiFi.macAddress());
        debug1("IP address: ");
        Serial.println(WiFi.localIP());
        debug1("WiFi Signal strength: %i dB\n\n",(WiFi.RSSI()));
        return true;
    } else {
        debug1("\nCould not connect to WIFI network!\n");
        // delay(5000);
        // ESP.reset(); // Reset and try again
        // ESP.restart(); // Reset and try again
        return false;
    }
  }
}