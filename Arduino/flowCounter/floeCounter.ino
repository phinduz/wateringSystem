int flowPin = 3;    //This is the input pin on the Arduino
int ledPin = 13;
volatile int count; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.  
int incomingByte;
int data;
String stringCommand = "";         // a string to hold incoming data
boolean newCommand = false;  // whether the string is complete
int number;
  
void setup() {
  // put your setup code here, to run once:
  pinMode(flowPin, INPUT);           //Sets the pin as an input
  pinMode(ledPin, OUTPUT);           
  attachInterrupt(1, Flow, RISING);  //Configures interrupt 0 (pin 3 on the Arduino Uno) to run the function "Flow"  
  Serial.begin(9600);  //Start Serial
  count = 0;      // Reset the counter so we start counting from 0 again
  interrupts();   //Enables interrupts on the Arduino

}
void loop() {
  // put your main code here, to run repeatedly:  

  delay (1000);   //Wait 1 second 

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
      Serial.println(number);
      Serial.println(data);
    }
    if (stringCommand[0] == 'r') {
      // Do the read function
      Serial.print('r');
      Serial.println(number);
      Serial.println(count);
      //Serial.println(data);
    }
    
    // clear the string:
    stringCommand = "";
    newCommand = false;
   }
  
  if (count > 20) {
    digitalWrite(ledPin, HIGH);
  }

  //Serial.println(count);         //Print the variable flowRate to Serial
}

void Flow()
{
   count++; //Every time this function is called, increment "count" by 1
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the stringCommand:
    stringCommand += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      newCommand = true;
    }
  }
}
