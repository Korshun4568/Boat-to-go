// Include iBusBM Library
#include <IBusBM.h>

// Note IBusBM library labels channels starting with "0"

// Create iBus Object
IBusBM ibus;
String M = "";
// Read the number of a given channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch = ibus.readChannel(channelInput);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

// Read the channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue) {
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}
void setup() {
  // Start serial monitor
  //Serial.begin(115200);

  // Attach iBus object to serial port
  Serial.begin(9600);
  ibus.begin(Serial1);
  Serial3.begin(9600);
  delay(10000);
}

void loop() {

  int State = readChannel(8, 0, 2, 1);
  //Типа 2 это ручной рержим, 0 это нейтралка, 1 это автономка
  if (State == 2) {
    int rule = readChannel(3, -500, 500, 0);
    int sp = readChannel(1, -500, 500, 0);
    //  Serial.print(rule);
    //  Serial.print("    ");
    //  Serial.println(sp);
    M = String(sp + 2000) + String(sp + 2000) + String(rule * -1 + 2000) + String(rule + 2000) + "$";

    // M = "1000100010001000$";
    Serial3.print(M);


    delay(10);
  }
  Serial.println(State);

}
