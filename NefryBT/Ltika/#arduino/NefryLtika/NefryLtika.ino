//NefryについているLEDを光らせる専用の関数がある
//遅延させる関数もあり、ほかの処理とは非同期に動く
#include <Nefry.h>

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  Nefry.setLed(255,0,0);
  Nefry.print("[LED ON ]");
  Nefry.println(millis()/1000.0f);
  Nefry.ndelay(1000);
  
  Nefry.setLed(0,0,0);
  Nefry.print("[LED OFF]");
  Nefry.println(millis()/1000.0f);
  Nefry.ndelay(1000);
  
}
