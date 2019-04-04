#include <Wire.h>
#define PIN_SDA 17//D1
#define PIN_SCL 5//D0
void setup() {
  Serial.begin(115200);
  Serial.println("[Start] wire.begin()");
  Wire.begin(); //Wire.begin(PIN_SDA,PIN_SCL); //(SDA, SCL)
  Serial.println("[End] wire.begin()");
}

void loop() {
  Serial.println("LOOP");
}
