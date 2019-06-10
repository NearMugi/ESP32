#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
    Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

//pulse
#define PIN_PULSE D0
bool sw;

void setup()
{
    pinMode(PIN_PULSE, OUTPUT);
    sw = false;
}

void loop()
{
    digitalWrite(PIN_PULSE, sw);
    sw = !sw;
}
