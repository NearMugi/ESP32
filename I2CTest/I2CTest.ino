#include <Nefry.h>
#include "Wire.h"
#define SDA D1
#define SCL D0

void setup() {
  pinMode(SDA, INPUT_PULLUP); //デファルトのSDAピン21　のプルアップの指定
  pinMode(SCL, INPUT_PULLUP); //デファルトのSDAピン22　のプルアップの指定
  Wire.begin(SDA, SCL);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("LOOP");
}
