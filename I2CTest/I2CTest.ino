//https://github.com/Nefry-Community/arduino-esp32/tree/fix/i2c
//NefryBTでI2C通信ができるようにしたパッケージが必要だった。

#include <Nefry.h>
#include "Wire.h"

void setup() {
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Nefry.println("LOOP");
}
