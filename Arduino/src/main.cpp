#include <Arduino.h>
#include <Wire.h>
#include "FooLib.h"


// Create an object of Foo
FooClass FooObject;

// Forward declaration
int parseCommand(char * str);
void serialEvent();


boolean new_command = false;  // Tells if the string is complete
const int num_chars = 64;
char received_chars[num_chars];   // an array to store the received data
static int index = 0;
char in_char;

boolean any_response = false;


void setup() {
    // Start Serial communication to Raspberry Pi
    Serial.begin(57600, SERIAL_8E1);
    //delay(1000);
    Serial.flush();
	//delay(1000);
}


int led_pin = 13;

void blink(int time_on, int time_off)
{
    digitalWrite(led_pin, HIGH);
    delay(time_on);
    digitalWrite(led_pin, LOW);
    delay(time_off);
}


void loop() {
    if (!any_response) {
        blink(1, 200); 
    }

    if (new_command) {
        blink(100, 100);
        // parseCommand(received_chars);
        Serial.println(received_chars);
        //Serial.println("123456");

        //http://www.cplusplus.com/reference/cstring/strtok/
        //https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
        new_command = false;
    }    
}

// Handle serial input
void serialEvent() {
    any_response = true;
    while (Serial.available() && !new_command) {
        char in_char = (char)Serial.read();

        if (in_char != '\n') {
            received_chars[index] = in_char;
            index++;
            // Avoid buffer overflow by skipping subsequent chars
            if (index >= num_chars) {
                index = num_chars - 1;
            }
        }
        else {
            received_chars[index] = '\0';
            index = 0;
            new_command = true;
        }
    }
}

int parseCommand(char * str)
{
    str++;
    return 0;
}