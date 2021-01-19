#include <Arduino.h>
#include "DPS310Height.h"
#include <Dps310.h>
#include "amedas.h"

Dps310 dps310_ = Dps310();

void DPS310Height::connect()
{
    Wire.begin(26, 32);
    delay(10);
    dps310_.begin(Wire, 0x77);
};

void DPS310Height::setBasePressure()
{
    // 気象庁データを取得
    amedas amedasData(posID);
    amedasData.getData();
    baseP = amedasData.getLastPressure();
    baseT = amedasData.getLastTemperature();

    // 海面気圧を算出する
    // https://keisan.casio.jp/exec/system/1203302206
    seaLevelPressure = baseP * pow(1 - ((0.0065 * baseH) / (baseT + 0.0065 * baseH + 273.15)), -5.257);
    // 逆算する
    heightBase = ((pow((seaLevelPressure / baseP), powV) - 1) * (baseT + 273.15)) / 0.0065;
};
void DPS310Height::update()
{
    isGood = false;
    bool ret = dps310_.measurePressureOnce(pressure, oversampling);
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

    // https://keisan.casio.jp/exec/system/1257609530
    heightBaseTempe = ((pow((seaLevelPressure / pressure), powV) - 1) * (baseT + 273.15)) / 0.0065;
    heightDPS310 = ((pow((seaLevelPressure / pressure), powV) - 1) * (temperature + 273.15)) / 0.0065;

    heightDPS310_LPF = (1 - LPF_k) * heightDPS310_LPFLast + LPF_k * heightDPS310;
    heightDPS310_LPFLast = heightDPS310_LPF;

    isGood = true;
};
