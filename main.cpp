#include <Arduino.h>
#include <IBusBM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

const int compassAddr = 0x01;
const byte M[8] = {0b11000011,0b11000011,0b11000011,0b11011011,0b11111111,0b11111111,0b11100111,0b11000011};
const byte A[8] = {0b01100110,0b01100110,0b01111110,0b01111110,0b01100110,0b01100110,0b01111110,0b00111100};
const byte G[8] = {0b11111111,0b10100101,0b11100111,0b00100100,0b00100100,0b00111100,0b00100100,0b00011000};
const byte N[8] = {0b11000011,0b11000111,0b11001111,0b11011111,0b11111011,0b11110011,0b11100011,0b11000011};
const byte B[8] = {0b00011110,0b00111111,0b10111111,0b10111111,0b10111111,0b10011110,0b01001100,0b00111000};

IBusBM ibus;
Max72xxPanel matrix = Max72xxPanel(53, 1, 1);

String nano;
int state;
int kp, kd, ki;
int err, speed, goal, azimuth;
bool boolgun = false, boolbomb = false;



int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
    uint16_t ch = ibus.readChannel(channelInput);
    if (ch < 100) return defaultValue;
    return map(ch, 1000, 2000, minLimit, maxLimit);
}

bool key() {
    return digitalRead(A0);
}

void mat() {
    matrix.fillScreen(LOW);

    if (state == 0) for (int i = 0; i < 8; i++) for (int j = 0; j < 8; ++j) matrix.drawPixel(j, i, N[i] & (1 << j));
    if (state == 1) for (int i = 0; i < 8; i++) for (int j = 0; j < 8; ++j) matrix.drawPixel(j, i, A[i] & (1 << j));
    if (state == 2) {
        if     (boolbomb) for (int i = 0; i < 8; i++) for (int j = 0; j < 8; ++j) matrix.drawPixel(j, i, B[i] & (1 << j));
        else if(boolgun ) for (int i = 0; i < 8; i++) for (int j = 0; j < 8; ++j) matrix.drawPixel(j, i, G[i] & (1 << j));
        else              for (int i = 0; i < 8; i++) for (int j = 0; j < 8; ++j) matrix.drawPixel(j, i, M[i] & (1 << j));
    }

    matrix.write();
}

void compass() {
    Wire.beginTransmission(compassAddr);
    Wire.write(0x44);
    Wire.endTransmission();

    Wire.requestFrom(compassAddr, 2);
    while (Wire.available() < 2);

    byte lowbyte = Wire.read();
    byte highbyte = Wire.read();

    azimuth = word(highbyte, lowbyte);
}

void pid() {
    nano = "";

    int u = 0;

    nano = String(speed) // m1
           + String(speed) // m2
           + String(u)     // m3
           + String(u);    // m4
}

void neutral() {
    nano = "200020002000200000";
}

void rpi() {
    if(Serial2) {
        String rpi = Serial2.readStringUntil('$');

        if (rpi[0] == 0) { // default mode
            err   =            rpi.substring(1, 4).toInt() - 180,
                    speed = 2000 + map(rpi.substring(4, 7).toInt() - 100, 0, 100, -500, 500);

            pid();
        }

        if (rpi[0] == 1) { // omni mode
            nano = String(2000 + map(rpi.substring(1, 4 ).toInt() - 100, 0, 100, -500, 500))  // m1
                   + String(2000 + map(rpi.substring(4, 7 ).toInt() - 100, 0, 100, -500, 500))  // m2
                   + String(2000 + map(rpi.substring(7, 10).toInt() - 100, 0, 100, -500, 500))  // m3
                   + String(2000 + map(rpi.substring(10,13).toInt() - 100, 0, 100, -500, 500)); // m4
        }

        if (rpi[0] == 2) { // compass mode
            compass();

            goal  = 2000 + map(rpi.substring(1, 4).toInt() - 100, 0, 100, -500, 500),
                    speed = 2000 + map(rpi.substring(4, 7).toInt() - 100, 0, 100, -500, 500);
            err = goal - err;

            pid();
        }
    }
}

void manual() {
    err   =        readChannel(3, -500, 500, 0),
            speed = 2000 + readChannel(1, -500, 500, 0);

    nano = String(speed) + String(speed) + String(err * -1 + 2000) + String(err + 2000);
}

void debug() {
    String str = "Mode: ";
    state == 0 ? str += "Neutral\n" : state == 1 ? str += "Autonomous\n" : str += "Manual\n";

    str += "Azimuth: " + String(azimuth);

    Serial2.print(str + "$");
}

void setup() {
    ibus.begin(Serial1);     // receiver

    matrix.setIntensity(2); // brightness
    matrix.setRotation (2); // rotation

    Wire.begin();
    Wire.beginTransmission(compassAddr);
    Wire.write(0x01);
    Wire.endTransmission();
    while (Wire.available() > 0) Wire.read();

    Serial .begin(115200); // monitor
    Serial2.begin(115200); // rpi
    Serial3.begin(115200); // nano

    pinMode(A0, INPUT_PULLUP);

    // delay(10000);           // esc init delay
}



void loop() {
    key() ?
    state    = readChannel(8, 0, 2, 1) : state = 1; // 0 auto 1 manual
    boolgun  = readChannel(7, 0, 1, 1);
    boolbomb = readChannel(6, 0, 1, 1);

    mat();
    compass();

    if (state == 0) neutral(); // neutral mode
//     if (state == 1) rpi();     // rpi mode
    if (state == 2) manual();  // manual mode

    Serial.println(azimuth);
    Serial3.println(nano + "$");
    debug();
}