#ifndef loadCell_H
#define loadCell_H

class loadCell
{
public:
    void init(int _pin_dout, int _pin_slk, float _outVolt, float _load);
    float getData();

private:
    void AE_HX711_Init(void);
    void AE_HX711_Reset(void);
    long AE_HX711_Read(void);
    long AE_HX711_Averaging(long adc, char num);
    float AE_HX711_getGram(char num);

    float offset;
    int pin_dout;
    int pin_slk;

    //定格出力 [V]
    float OUT_VOL;
    //定格容量 [g]
    float LOAD;
};

#endif