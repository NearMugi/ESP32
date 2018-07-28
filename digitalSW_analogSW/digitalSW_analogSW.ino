#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

#define PIN_DIGITAL_SW D8
#define PIN_ANALOG_SW A3


void setup() {
  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  // put your setup code here, to run once:
  pinMode(PIN_DIGITAL_SW, INPUT);
  pinMode(PIN_ANALOG_SW, INPUT);

}

void loop() {
  NefryDisplay.clear();
  NefryDisplay.setFont(ArialMT_Plain_16);
  NefryDisplay.drawString(0, 0, String(digitalRead(PIN_DIGITAL_SW)));
  NefryDisplay.drawString(0, 20, String(analogRead(PIN_ANALOG_SW)));
  NefryDisplay.display();
  Nefry.ndelay(20);
}
