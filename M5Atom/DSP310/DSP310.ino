#include <Arduino.h>
#include "env.h"
#include <WiFi.h>
#include <WiFiMulti.h>

#include <memory>
#include "m5AtomBase.h"
#include <Dps310.h>
#include "amedas.h"
Dps310 dps310_ = Dps310();
m5AtomBase m5Atom;

WiFiMulti WiFiMulti;
const String posID = "44132";
amedas amedasData(posID);

// https://www.jma.go.jp/jp/amedas_h/today-44132.html
const float baseH = 25.0;
float baseP = 1013.0;
float baseT = 0.0;

// 海面気圧
float p0;

void setup()
{
    M5.begin(true, false, true);
    // connect to dps310
    Wire.begin(26, 32);
    delay(10);
    dps310_.begin(Wire, 0x77);

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

    amedasData.getData();
    baseP = amedasData.getLastPressure();
    baseT = amedasData.getLastTemperature();

    // 海面気圧を算出する
    // https://keisan.casio.jp/exec/system/1203302206
    p0 = baseP * pow(1 - ((0.0065 * baseH) / (baseT + 0.0065 * baseH + 273.15)), -5.257);

    m5Atom.LED.setWhite();
}

void loop()
{
    float temperature;
    float pressure;
    int16_t oversampling = 7;
    int16_t ret;

    m5Atom.LED.setBlue();
    ret = dps310_.measurePressureOnce(pressure, oversampling);
    if (ret != 0)
    {
        Serial.println("pressure fail! ret = " + ret);
        return;
    }
    pressure = pressure / 100.0;

    ret = dps310_.measureTempOnce(temperature, oversampling);
    if (ret != 0)
    {
        Serial.println("temperature fail! ret = " + ret);
        return;
    }

    m5Atom.LED.setGreen();
    // https://keisan.casio.jp/exec/system/1257609530
    float baseHeight = ((pow((p0 / baseP), 1.0 / 5.257) - 1) * (baseT + 273.15)) / 0.0065;
    float height = ((pow((p0 / pressure), 1.0 / 5.257) - 1) * (baseT + 273.15)) / 0.0065;
    float ofsHeight = height - baseHeight;

    String _tmp = String(temperature);
    _tmp += ", ";
    _tmp += String(pressure);
    _tmp += ", ";
    _tmp += String(height, 4);
    _tmp += ", ";
    _tmp += String(baseHeight, 4);
    _tmp += ", ";
    _tmp += String(ofsHeight, 4);
    Serial.println(_tmp);

    delay(100);
}
