
boolean new_command = false;  // Tells if the string is complete
const int num_chars = 64;
char received_chars[num_chars];   // an array to store the received data
static int index = 0;
char in_char;

void setup() {
    // Start Serial communication to Raspberry Pi
    Serial.begin(57600, SERIAL_8E1);
    Serial.flush();
}

void loop() {
    if (new_command) {
        Serial.println(received_chars);

        //http://www.cplusplus.com/reference/cstring/strtok/
        //https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
        new_command = false;
    }
}

// Handle serial input
void serialEvent() {
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
