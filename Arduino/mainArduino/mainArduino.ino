int flowPin = 3;    //This is the input pin on the Arduino
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

volatile int count; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.  

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
        
  // Not really confident on how the attachInterrupt function works here, but it seems to do the job 
  attachInterrupt(1, Counter, RISING);  
  Serial.begin(9600);  //Start Serial
  interrupts();   //Enables interrupts on the Arduino 
  
}
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

    // Switch between commands
    if (String("water") == command) {
      // Call command water with arguments
      // Maybe also write something back on serial to confirm command
      //Serial.print("");
      Water(commandValue[0], commandValue[1], commandValue[2]);
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
    newCommand = false;
  }
}

void Counter() {
   count++; //Every time this function is called, increment "count" by 1
}

int Water(int num,int amount,int type) {
  count = 0;
  previousTime = millis();
  currentTime = millis();
  if (type == 0) {
    // Use time in seconds as amount
    Serial.println("Water seconds...");
    // Start motor and open relay
    analogWrite(motor,255);
    analogWrite(relay[num],255);
    
    while ((currentTime - previousTime) < amount*1000) {
      //Wait until the desired amount is reached
      if (count < 10 && currentTime - previousTime > 3000) {
        // If the counter didn't increase at all after 3 s something is wrong 
          analogWrite(motor,0);
          analogWrite(relay[num],0);
          return -1;
      }
      currentTime = millis();
    }
  } else if (type == 1) {
    // Use number of centiliters as limit
    Serial.println("Water centiliters...");
    // Start motor and open relay
    analogWrite(motor,255);
    analogWrite(relay[num],255);
    while (count/countsPerCentiliter < amount) {
      //Wait until the desired amount is reached
      if (count < 10 && (currentTime - previousTime) > 3000) {
        // If the counter didn't increase at all after 3 s something is wrong 
          analogWrite(motor,0);
          analogWrite(relay[num],0);
          return -1;
      }
      currentTime = millis();
    }
  } else if (type == 2) {
    // Use number of counts as limit
    Serial.println("Water counts...");
    // Start motor and open relay
    analogWrite(motor,255);
    analogWrite(relay[num],255);
    while (count < amount) {
      //Wait until the desired amount is reached
      if (count < 10 && currentTime - previousTime > 3000) {
        // If the counter didn't increase at all after 3 s something is wrong 
        analogWrite(motor,0);
        analogWrite(relay[num],0);
        return -1;
      }
      currentTime = millis();
    }
  } else {
    // type was not 0 or 1, error in input, don't do anything
    return -1;
  }
  analogWrite(motor,0);
  analogWrite(relay[num],0);
  return true;
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
    Serial.println(inChar);
    Serial.println(stringCommand);
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
