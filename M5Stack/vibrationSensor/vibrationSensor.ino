// GROVE振動センサー
#include "M5Atom.h"
#include <Wire.h>

#define SDA 26
#define SCL 32

void setup()
{
  M5.begin(true, true, true);
  Wire.begin(SDA, SCL);
  M5.dis.drawpix(0, 0xf00000);
}

void loop()
{
  M5.update();
  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0)
    {
      //Serial.print("I2C device found at address 0x");
      //if (address < 16)
      //  Serial.print("0");
      //Serial.print(address, HEX);
      //Serial.println("  !");
      Wire.requestFrom(address, 1);
      Serial.println(Wire.read());
    }
  }
}
