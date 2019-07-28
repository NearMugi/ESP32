#include <Nefry.h>
#include <NefryDisplay.h>
#include <Adafruit_AMG88xx.h>

#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

const int pixel_array_size = 8*8;
float pixels[pixel_array_size];

Adafruit_AMG88xx amg;
bool status;

void setup() {

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    status = amg.begin(0x69);
}

void loop() {
    if(!status) {
        Serial.println("Connect Error...");
        return;
    }

    amg.readPixels(pixels);

    String t = "";
    for(int i=0; i<pixel_array_size; i++){
        t += String(pixels[i]);
        t += ",";
    }
    Serial.println(t);
    delay(100);
}