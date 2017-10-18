/*
  Water.cpp - Library for running a water pump
*/

#include "Arduino.h"
#include "Water.h"

Water::Water(int motor,int relayNum,int flowPin,int amount,int type,int countsPerCentiliter = 1)
{
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

  if (_type == 0) {
    Serial.println("Water seconds...");
    seconds();
    return 1;
  } else if (_type == 1) {
    Serial.println("Water counts...");
    return amountOfCentiliters();
  } else if (_type == 2) {
    Serial.println("Water centiliters...");
    return numberOfcounts();
  } else {
    // type was not 0 or 1, error in input, don't do anything
    detachInterrupt(_flowPin);
    noInterrupts();
    return -1;
  }

  detachInterrupt(_flowPin);
  noInterrupts();
  analogWrite(_motor,0);
  analogWrite(_relayNum,0);
  return true;
}

void Water::counter() {
  count++; //Every time this function is called, increment "count" by 1
}

bool Water::statusOK() {
  if (count < 10 && currentTime - previousTime > 3000) {
    // If the counter didn't increase at all after 3 s something is wrong 
      analogWrite(_motor,0);
      analogWrite(_relayNum,0);
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
      return -1;
    }
    currentTime = millis();
  } 
}

int Water::amountOfCentiliters() {
  // Use _number of centiliters as limit
  // Start motor and open relay
  analogWrite(_motor,255);
  analogWrite(_relayNum,255);
  while (count/_countsPerCentiliter < _amount) {
    //Wait until the desired amount is reached
    if ( !statusOK() ) {
      return -1;
    }
    currentTime = millis();
  }
}

int Water::numberOfcounts() {
  // Use _number of counts as limit
  // Start motor and open relay
  analogWrite(_motor,255);
  analogWrite(_relayNum,255);
  while (count < _amount) {
    //Wait until the desired _amount is reached
    if ( !statusOK() ) {
      return -1;
    }
    currentTime = millis();
  }
}
