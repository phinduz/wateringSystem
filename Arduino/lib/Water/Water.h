/*
  Water.h - Library for running a water pump
*/
#ifndef Water_h
#define Water_h

#include "Arduino.h"

class Water
{
  public:
    Water(int motor,int relayNum,int flowPin,int amount,int type,int countsPerCentiliter);
    int run();
  private:
    static void counter();
    bool statusOK();
    int seconds();
    int amountOfCentiliters();
    int numberOfcounts();
    int _motor;
    int _relayNum;
    int _flowPin;
    int _amount;
    int _countsPerCentiliter;
    int _type;
    int previousTime;
    static volatile int count; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.     
    int currentTime;
};

#endif
