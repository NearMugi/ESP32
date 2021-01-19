#include <Arduino.h>
#include "env.h"
#include <WiFi.h>
#include <WiFiMulti.h>

#include <memory>
#include "m5AtomBase.h"
#include "DPS310Height.h"

WiFiMulti WiFiMulti;
m5AtomBase m5Atom;
DPS310Height dps310Height;

void setup()
{
    M5.begin(true, false, true);

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid, pw);

    // wait for WiFi connection
    Serial.print("Waiting for WiFi to connect...");
    while ((WiFiMulti.run() != WL_CONNECTED))
    {
        Serial.print(".");
    }
    Serial.println(" connected");

    // connect to dps310
    dps310Height.connect();
    dps310Height.setBasePressure();

    m5Atom.LED.setWhite();
}

void loop()
{
    m5Atom.LED.setBlue();
    dps310Height.update();

    m5Atom.LED.setGreen();
    if (dps310Height.getIsGood())
    {
        String _tmp = "";
        //_tmp += String(dps310Height.getTemperature());
        //_tmp += ", ";
        //_tmp += String(dps310Height.getPressure());
        //_tmp += ", ";
        _tmp += String(dps310Height.getHeightBase(), 4);
        _tmp += ", ";
        _tmp += String(dps310Height.getHeightBaseTempe(), 4);
        _tmp += ", ";
        _tmp += String(dps310Height.getHeightDPS310(), 4);
        _tmp += ", ";
        _tmp += String(dps310Height.getHeightDPS310_LPF(), 4);
        Serial.println(_tmp);
    }
    delay(100);
}
