#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
    Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_PULSE_160 1000000
#define InterruptTime 1000

//debug
unsigned long countTime;
unsigned long lpTime;
bool sw;

//pulse
#define PIN_PULSE D0
float pulse160_interrupt = 3125;

class haptic
{
public:
    haptic(float p)
    {
        //1セット(On,Offを3回＋50msの待ち)
        pulse[0] = p;
        pulse[1] = p;
        pulse[2] = p;
        pulse[3] = p;
        pulse[4] = p;
        pulse[5] = p + (50000 / (float)InterruptTime);
    }
    bool getisOn()
    {
        return isOn;
    }
    bool getisPlay()
    {
        if (idx <= 5)
            return true;
        return false;
    }

    String debugMsg()
    {
        String s = "";
        if (isOn)
        {
            s += "ON, ";
        }
        else
        {
            s += "OFF, ";
        }

        s += String(idx) + ", ";
        s += String(remainTime);

        return s;
    }
    void start()
    {
        isPlay = true;
        idx = 0;
        isOn = true;
        remainTime = pulse[idx];
    }

    //割込み処理で呼ばれる
    void countDown()
    {
        if (!isPlay)
            return;

        remainTime -= InterruptTime;
        if (remainTime < 0)
        {
            if (++idx <= 5)
            {
                remainTime = pulse[idx];
                isOn = !isOn;
            }
            else
            {
                //1セット終了
                isPlay = false;
            }
        }
    }

private:
    volatile bool isPlay;
    volatile bool isOn;
    volatile int idx;
    volatile float pulse[6];
    volatile int remainTime;
};

haptic ptn160 = haptic(pulse160_interrupt);

void setup()
{
    pinMode(PIN_PULSE, OUTPUT);

    Nefry.enableSW();

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    sw = false;
}

void loop()
{
    digitalWrite(PIN_PULSE, sw);
    sw = !sw;
}
