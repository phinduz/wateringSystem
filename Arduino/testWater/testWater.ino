#include <Water.h>

// Flow sensor is physically connected to pin 3, which is interrupt pin 1
// Using the follwoing code should work, but it doesn't for some reason
// int flowPin = digitalPinToInterrupt(3)
int flowPin = 1;    //This is the input pin on the Arduino

int ledPin = 13;
int motor = 4;  //Digital pins
int relay[] = {5,6,7,8,9,10,11,12}; //Digital pins
int sensor[] = {A0,A1,A2,A3,A4,A5,A6,A7}; //Analog pins
int countsPerCentiliter = 236;
int maxTime = 30; // Will never allow to water for longer than 30 seconds

unsigned long previousTime = 0;
unsigned long currentTime = 0;

  
void setup() {
  // put your setup code here, to run once:
  pinMode(flowPin, INPUT);           //Sets the pin as an input
  pinMode(ledPin, OUTPUT);           //Set the led pin as output
  
  // Setup relay pins
  for (int k=0; k<8; k++) {
    pinMode(relay[k], OUTPUT);
  } 
  // Setup sensor pins
  for (int k=0; k<8; k++) {
    pinMode(sensor[k], OUTPUT);
  }
  
}

int amount = 10;
int type = 0;
int relayNum = relay[1];
int centiliter = 1;
int out;


//https://arduino.stackexchange.com/questions/20608/pass-classs-public-function-as-an-argument-to-external-command



Water waterObject(motor,relayNum,flowPin,amount,type,centiliter);

// The intterrupt counter has to be declared
volatile int Water::count;

void loop() {
  waterObject.run();
}

