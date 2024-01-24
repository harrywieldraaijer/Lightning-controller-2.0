#pragma Once
/*
    Created by: Harry Wieldraaijer


    Short description: This module contains all KlikAanKlikUit code and funtions
     
    Version history: 
    
      Version 0.1 - 20-januari-2024    Initial version

    Prerequisites: 
      None
      
*/

#include <NewRemoteTransmitter.h>  // required for KlikAanKlikUit
#define clickOnClickOffPin 17
#define clickOnClickOfAddress 19560623

namespace KlikAanKlikUit {

  NewRemoteTransmitter dimmer01 (clickOnClickOfAddress,clickOnClickOffPin,260,3); //dimmer
  NewRemoteTransmitter transmitter01 (clickOnClickOfAddress,clickOnClickOffPin,260,3); //switch

  void setDimmer(int locLevel){
    const int dimmerLevel = locLevel * 15  / 100 ;
    if (dimmerLevel==0) {
      transmitter01.sendUnit(0,false); // Because sendDim(0,0) does not dim to 0%
    }  else {
      dimmer01.sendDim(0,dimmerLevel);
    }
  }
}