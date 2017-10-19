/*
  Water.cpp - Library for running a water pump
*/

#include "Arduino.h"
#include "Water.h"

Water::Water(int motor,int relayNum,int flowPin,int amount,int type,int countsPerCentiliter) {
  _motor = motor;
  _relayNum = relayNum;
  _flowPin = flowPin;
  _amount = amount;
  _type = type;
  _countsPerCentiliter = countsPerCentiliter; 
  count = 0;
  previousTime = millis();
  currentTime = millis();
}

int Water::run() {
  //Enable interrupt on the flow sensor
  attachInterrupt(_flowPin,counter,RISING);  
  interrupts();
  count = 0;
  previousTime = millis();
  currentTime = millis();

  if (_type == 0) {
    Serial.println("Water seconds...");
    seconds();
  } else if (_type == 1) {
    Serial.println("Water counts...");
    amountOfCentiliters();
  } else if (_type == 2) {
    Serial.println("Water centiliters...");
    numberOfcounts();
  } else {
    // type was not 0, 1 or 2, error in input, don't do anything
    Serial.println("Unknown type");
    detachInterrupt(_flowPin);
    return 1;
  }
  Serial.println("DONE");
  // Turn off everything
  detachInterrupt(_flowPin);
  analogWrite(_motor,0);
  analogWrite(_relayNum,0);
  return 0;
}

void Water::counter() {
  count++; //Every time this function is called, increment "count" by 1
}

bool Water::statusOK() {
  if (count < 10 && currentTime - previousTime > 3000) {
    // If the counter didn't increase at all after 3 s something is wrong 
      analogWrite(_motor,0);
      analogWrite(_relayNum,0);
      Serial.println("Timeout after 3 s, no flow detected");
      return false;
  } else {
      return true;
  }
}

int Water::seconds() {
  // Use time in seconds as amount
  // Start motor and open relay
  analogWrite(_motor,255);
  analogWrite(_relayNum,255);
  while ((currentTime - previousTime) < _amount*1000) {
    //Wait until the desired _amount is reached
    if ( !statusOK() ) {
      return 1;
    }
    currentTime = millis();
  } 
  return 0;
}

int Water::amountOfCentiliters() {
  // Use _number of centiliters as limit
  // Start motor and open relay
  analogWrite(_motor,255);
  analogWrite(_relayNum,255);
  while (count/_countsPerCentiliter < _amount) {
    //Wait until the desired amount is reached
    if ( !statusOK() ) {
      return 1;
    }
    currentTime = millis();
  }
  return 0;
}

int Water::numberOfcounts() {
  // Use _number of counts as limit
  // Start motor and open relay
  analogWrite(_motor,255);
  analogWrite(_relayNum,255);
  while (count < _amount) {
    //Wait until the desired amount is reached
    if ( !statusOK() ) {
      return 1;
    }
    currentTime = millis();
  }
  return 0;
}
