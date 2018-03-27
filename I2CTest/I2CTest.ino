#include "Wire.h"

void setup() {
  Serial.begin(115200);
  Serial.println("[Start] wire.begin()");
  Wire.begin();
  Serial.println("[End] wire.begin()");
}

void loop() {
  Serial.println("LOOP");
}
