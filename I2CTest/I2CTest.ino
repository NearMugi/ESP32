//https://github.com/Nefry-Community/arduino-esp32/tree/fix/i2c
//https://esa-pages.io/p/sharing/3062/posts/943/63bac6a3d50b2f201cd9.html
//NefryBTでI2C通信ができるようにしたライブラリに変更が必要だった

#include <Nefry.h>
#include "Wire.h"

void setup() {
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Nefry.println("LOOP");
}
