int flowPin = 3;    //This is the input pin on the Arduino
int ledPin = 13;
int motor = 4;  //Digital pins
int relay[] = {5,6,7,8,9,10,11,12}; //Digital pins
int sensor[] = {A0,A1,A2,A3,A4,A5,A6,A7}; //Analog pins
int flowConstant = 1;

volatile int count; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.  
int data;
String stringCommand = "";         // a string to hold incoming data
boolean newCommand = false;  // whether the string is complete
int number;
  
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
        
  attachInterrupt(1, Counter, RISING);  //Configures interrupt 0 (pin 3 on the Arduino Uno) to run the function "Flow"  
  Serial.begin(9600);  //Start Serial
  count = 0;      // Reset the counter so we start counting from 0 again
  interrupts();   //Enables interrupts on the Arduino 
  
}
void loop() {
  // put your main code here, to run repeatedly:

  number = 0;
  data = 0;  


  // print the string when a newline arrives:
  // Expect command like w,3,5567, where w or means read/write input/output number and value
  if (newCommand) {
    if (stringCommand[1] == ',') {
      if (stringCommand[2] >= 48 && stringCommand[2] <= 57) {    //is this serial byte the ASCII equivalent of 0 through 9?
        number = (int)stringCommand[2]-48;
      }
    }
    if (stringCommand[3] == ',') {
      data = 0;
      for (int dataInd = 4; dataInd < stringCommand.length()-1; dataInd++) {
        data = 10*data + (int)stringCommand[dataInd]-48;
      }
    }
    if (data > 32767) {
      data = 0;
    }
      
    if (stringCommand[0] == 'w') {
      // Do the write function
      Serial.print('w');
      Water(number,data);
      Serial.print(number);
      Serial.print(data);
    }
    if (stringCommand[0] == 'r') {
      // Do the read function
      Serial.print('r');
      Serial.print(ReadSensor(number));
    }
    
    // clear the string:
    stringCommand = "";
    newCommand = false;
   }
}

void Counter() {
   count++; //Every time this function is called, increment "count" by 1
}

boolean Water(int num,int amount) {
  count = 0;
  //int timer = millis();
  analogWrite(motor,255);
  analogWrite(relay[num],255);
  while (count*flowConstant < amount) {
    //Wait until the desired amount is reached
  }
  analogWrite(motor,0);
  analogWrite(relay[num],0);
  return true;
}

int ReadSensor(int num) {
  return analogRead(num);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the stringCommand:
    stringCommand += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == 'f') {
      newCommand = true;
    }
  }
}
