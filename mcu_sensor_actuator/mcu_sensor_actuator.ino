/*
  Recieves and executes command from the serial port.
*/

#include <ArduinoJson.h>
#include "I2Cdev.h"
#include "Wire.h"

// Serial port communication related
char in_char; // incoming single char from serial buffer
char input_chars[100] = "\0"; // char pointer to hold incoming serial data
bool char_array_complete = false; // flag when the input char array is complete
const int capacity = JSON_OBJECT_SIZE(16); // capacity of json document, related to maximum number objects
StaticJsonDocument<capacity> doc; // json document

// Keys and values in json string
char COMMAND[2] = "c";
char ARGUMENTS[5] = "args";
char VALUE[6] = "value";
char AMOUNT[2] = "a";
char RELAY[3] = "r";
char PULSES[3] = "ps";
char TIME[2] = "t";
char READ[5] = "read";
char WRITE[6] = "write";
char PUMP[2] = "p";
char INIT[5] = "init";
char ANALOG[2] = "a";
char DIGITAL[2] = "d";
char I2C[4] = "i2c";

int analog_array[8] = {A0, A1, A2, A3, A4, A5, A6, A7}; // array with analog pins
volatile int pulses; // keep track of pulses from flow sensor
int max_time = 18; // maximum time allowed for pumping

// The setup routine runs once when you press reset
void setup() {
  // Initialize serial port
  Serial.begin(115200, SERIAL_8E1);
  delay(100);
  Serial.flush();
  reset_pins(); // reset all pins
  // interrupt pin must be hardcoded at compile time
  attachInterrupt(digitalPinToInterrupt(2), counter, RISING);
}

// Reset output for all pins
void reset_pins() {
  for (int i = 0; i <= 13; ++i) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
    if (i <= 7) {
      pinMode(analog_array[i], OUTPUT);
      analogWrite(analog_array[i], LOW);
    }
  }
  pinMode(2, INPUT);
}

// The loop routine runs forever
void loop() {
  // Parse the input command when received
  if (char_array_complete) {
    runCommand(input_chars);
    // Clear input char array
    input_chars[0] = '\0';
    char_array_complete = false;
    //reset_pins(); // reset all pins after command is run
  }

  // The delay gives the serial handler time to send/receive data and must be set carefully.
  // If this delay is set too low it will not have enough time to send/receive and data will be lost.
  delay(50);
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  int index = 0;
  while (Serial.available()) {
    in_char = (char)Serial.read();
    input_chars[index++] = in_char;
    // \n marks end of the command
    if (in_char == '\n') {
      char_array_complete = true;
    }
  }
}

// Read value from specified IO address
int readValue(char *io_class, int address, int reg=0) {
  int value = -1;
  if (strcmp(io_class, ANALOG) == 0) {
    pinMode(analog_array[address], INPUT);
    value = analogRead(analog_array[address]);
  } else if (strcmp(io_class, DIGITAL) == 0) {
    pinMode(address, INPUT);
    value = digitalRead(address);
  } else if (strcmp(io_class, I2C) == 0) {
    
  } else {
    
  }
  return value;
}

// ISR that counts the number of pulses from flow sensor
void counter() {
  ++pulses; //Every time this function is called, increment "pulses" by 1
}

// Start pump and open corresponding relay
void startPump(int pump, int relay) {
  pinMode(relay, OUTPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(relay, HIGH);
  delay(50);
  digitalWrite(pump, HIGH);
}

// Stop pump and close corresponding relay
void stopPump(int pump, int relay) {
  digitalWrite(pump, LOW);
  delay(50);
  digitalWrite(relay, LOW);
}

// Handle pumping for specified amount/time
int runPump(int pump, int relay, int mode, int amount) {

  int limit;
  int max_limit;
  // Reset pulse counter
  pulses = 0;
  int count;
  startPump(pump, relay);
  max_limit = millis() + max_time * 1000; // maximum allowed time
  // Time mode
  if (mode == 0) {
    limit = millis() + amount * 1000; // specified time limit
    while (millis() <= limit and millis() <= max_limit) {
      delay(5);
    }
  // Pulse mode
  } else if (mode == 1) {
    limit = pulses + amount; // specified pulse limit
    while (pulses <= limit and millis() <= max_limit) {
      delay(5);
    }
  }
  stopPump(pump, relay);
  // Cannot return pulses that is volatile
  count = pulses;
  return count;
}

// Run command when it has been received on serial bus
void runCommand(char *input_string) {
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, input_string);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Handle each command
  const char* cmd = doc[COMMAND];
  if (strcmp(cmd, PUMP) == 0) {    
    int amount = -1;
    int mode = -1;
    if (!doc[AMOUNT][TIME].isNull()) {
      amount = doc[AMOUNT][TIME];
      mode = 0;
    } else if (!doc[AMOUNT][PULSES].isNull()) {
      amount = doc[AMOUNT][PULSES];
      mode = 1;
    }
    int pulses = runPump(doc[PUMP][DIGITAL], doc[RELAY][DIGITAL], mode, amount); 
    doc[VALUE] = pulses; // report back the number of pulses
  } else if (strcmp(cmd, READ) == 0) {
    doc[COMMAND] = "READ";    
    JsonArray args = doc[ARGUMENTS];
    int max_index = 16;
    int value = -1;
    for (int i=0; i<max_index; ++i) {
      if (!args[i].isNull())
      {
        JsonObject arg = args[i];
        if (!arg[ANALOG].isNull()) {
          arg[VALUE] = readValue(ANALOG, arg[ANALOG]);
        } else if (!arg[DIGITAL].isNull()) {
          arg[VALUE] = readValue(DIGITAL, arg[DIGITAL]);
        } else if (!arg[I2C].isNull()) {
          arg[VALUE] = readValue(I2C, arg[I2C][0], arg[I2C][1]);
        } else {
          
        }
      } else {
        i = max_index;
      }
    }
  } else if (strcmp(cmd, WRITE) == 0) {
    doc[COMMAND] = "WRITE";
  } else if (strcmp(cmd, INIT) == 0) {
    doc[COMMAND] = "INIT";
  } else {
    doc[COMMAND] = "ELSE";
  }

  // Send back the command with additions:
  serializeJson(doc, Serial);
  Serial.println("");
  return;
}
