#ifndef DPS310_HEIGHT_H
#define DPS310_HEIGHT_H

class DPS310Height
{
public:
    DPS310Height() : posID("44132"), baseH(25.0), baseP(1013.0), baseT(0.0), seaLevelPressure(0.0), oversampling(7), isGood(false), heightDPS310_LPFLast(0.0), LPF_k(0.1), powV(1.0 / 5.257){};
    virtual ~DPS310Height(){};
    void connect();
    void setBasePressure();
    void update();
    bool getIsGood() const { return isGood; };
    float getTemperature() const { return temperature; };
    float getPressure() const { return pressure; };
    float getHeightBase() const { return heightBase; };
    float getHeightBaseTempe() const { return heightBaseTempe; };
    float getHeightDPS310() const { return heightDPS310; };
    float getHeightDPS310_LPF() const { return heightDPS310_LPF; };

private:
    // 気象庁データから算出した基準とする気圧・気温
    // https://www.jma.go.jp/jp/amedas_h/today-44132.html
    const String posID;
    const float baseH;
    float baseP;
    float baseT;

    // 海面気圧
    float seaLevelPressure;

    // DPS310
    bool isGood;
    float temperature;
    float pressure;
    const int16_t oversampling;

    // Height
    // 基準値の高さを再計算(確かめ用)
    float heightBase;
    // 基準値の気温から算出
    float heightBaseTempe;
    // DPS310の気温から算出
    float heightDPS310;
    // DPS310の気温から算出(ローパスフィルタ)
    float heightDPS310_LPF;
    float heightDPS310_LPFLast;
    float LPF_k;

    // calc
    const float powV;
};

#endif