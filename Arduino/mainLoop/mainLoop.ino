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
String command = ""; // Contains the command that is set to be run
char startFlag = '('; // Marks start of serial command
char stopFlag = ')'; // Marks end of serial command
String tempValue;
int commandValue[] = {-1, -1, -1, -1}; // Contains values for commands
int commaIndex[] = {0, 0}; // Is used to store index of substrings
String stringCommand = ""; // String to hold incoming data
boolean newCommand = false;  // Tells if the string is complete

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
  //Start Serial communication to Raspberry Pi
  Serial.begin(9600);    
}

int amount = 10;
int type = 0;
int out;

// The intterrupt counter has to be declared
//https://arduino.stackexchange.com/questions/20608/pass-classs-public-function-as-an-argument-to-external-command
volatile int Water::count;

void loop() {
  if (newCommand) {
    
    // Parsing the string containg command and values 
    commaIndex[0] = 1;
    commaIndex[1] = stringCommand.indexOf(',');
    // stringCommand should look like (command,value0,value1,value2,value3)
    command = stringCommand.substring(commaIndex[0], commaIndex[1]);
    commaIndex[0] = commaIndex[1]+1;
    commaIndex[1] = stringCommand.indexOf(',', commaIndex[0]);
    int valueInd = 0;
    while (commaIndex[1] > 0) {
      if (commaIndex[1] > 0) {
        tempValue = stringCommand.substring(commaIndex[0], commaIndex[1]);
        commandValue[valueInd] = tempValue.toInt();
      }
      valueInd = valueInd+1;
      commaIndex[0] = commaIndex[1]+1;
      commaIndex[1] = stringCommand.indexOf(',', commaIndex[0]);
    }
    // Get the last value
    commaIndex[1] = stringCommand.indexOf(')', commaIndex[0]+1);
    tempValue = stringCommand.substring(commaIndex[0], commaIndex[1]);
    commandValue[valueInd] = tempValue.toInt();
    
    
    Serial.println(stringCommand);
    Serial.println(command);
    Serial.println(commandValue[0]);
    Serial.println(commandValue[1]);
    Serial.println(commandValue[2]);
    
    // Switch between commands
    if (String("water") == command) {
    // Call command water with arguments
      
      amount = commandValue[1];
      type = commandValue[0];
      
      Water waterObject(motor,relay[1],flowPin,amount,type,countsPerCentiliter);
      // The intterrupt counter has to be declared
      //https://arduino.stackexchange.com/questions/20608/pass-classs-public-function-as-an-argument-to-external-command
          
      waterObject.run();

//      Water(commandValue[0], commandValue[1], commandValue[2]);
    } else if (String("read") == command) {
      // Call command read with arguments and return sensor values
      Serial.println(ReadSensor(commandValue[0]));
    } else if (String("reset") == command) {
      // Restart the Arduino
      Serial.println("reset");
    } else if (String("setCent") == command) {
      // Set the centiliter constant
      Serial.println("setCent");
    } else {
      // The command was not understood, notify 
      Serial.print("Unknown command: ");
      Serial.println(command);
    }
    // clear the string:
    stringCommand = "";
    commandValue[0] = -1;
    commandValue[1] = -1;
    commandValue[2] = -1;
    commandValue[3] = -1;
    newCommand = false;
  }
}

// Read out value from sensor
int ReadSensor(int num) {
  return analogRead(num);
}

// Handle serial input
void serialEvent() {
  while (Serial.available() && !newCommand) {
    digitalWrite(ledPin, HIGH);
    // Get the new byte:
    char inChar = (char)Serial.read();
    //Serial.println(inChar);
    //Serial.println(stringCommand);
    // Ignore everything unless the first byte is the startFlag   
    if (stringCommand[0] == startFlag) {
      stringCommand += inChar;
    } else if (inChar == startFlag) {
      stringCommand = String(inChar);
    } else {
      stringCommand = "";
    }
    // if the incoming character is the stopFlag tell the main loop there are commands ready to execute
    if ((stringCommand[0] == startFlag) & (inChar == stopFlag)) {
      newCommand = true;
      digitalWrite(ledPin, LOW);
    }
  }
}

