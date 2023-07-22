#include <Servo.h>
Servo motor2;
Servo motor3;
Servo motor4;
Servo motor5;
String M = "";
void move(int m1, int m2, int m3, int m4) {
  motor2.writeMicroseconds(1490 + m1);
  motor3.writeMicroseconds(1490 + m2);
  motor4.writeMicroseconds(1490 + m3);
  motor5.writeMicroseconds(1490 - m4);

}
void setup() {
  // put your setup code here, to run once:
  // motor pwm pins
  motor2.attach(8, 1000, 2000);
  motor3.attach(9, 1000, 2000);
  motor4.attach(10, 1000, 2000);
  motor5.attach(11, 1000, 2000);
  delay(8000);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '$') {
      int m1 = M.substring(0, 4).toInt() - 2000;
      int m2 = M.substring(4, 8).toInt() - 2000;
      int m3 = M.substring(8, 12).toInt() - 2000;
      int m4 = M.substring(12, 16).toInt() -2000;
      move(m1, m2, m3, m4);
      M = "";
    } else {
      M += c;
      if (M.length() > 17)
        M = "";
    }
  }
}
