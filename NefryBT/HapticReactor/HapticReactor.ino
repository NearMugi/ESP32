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
#define LOOPTIME_GRAPH 10000
#define LOOPTIME_PULSE_160 500000

//割り込み処理
hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#define InterruptTime 10

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

void IRAM_ATTR onTimer()
{
    // Increment the counter and set the time of ISR
    portENTER_CRITICAL_ISR(&timerMux);
    isrCounter++;
    lastIsrAt = millis();
    portEXIT_CRITICAL_ISR(&timerMux);
    // Give a semaphore that we can check in the loop
    xSemaphoreGiveFromISR(timerSemaphore, NULL);
    // It is safe to use digitalRead/Write here if you want to toggle an output
}

class haptic
{
public:
    haptic(int p)
    {
        //1セット(On,Offを3回＋50msの待ち)
        pulse[0] = p;
        pulse[1] = p;
        pulse[2] = p;
        pulse[3] = p;
        pulse[4] = p;
        pulse[5] = p + 50000;
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

    void start()
    {
        idx = 0;
        isOn = true;
        remainTime = pulse[idx];
    }

    //1usごとに割込み処理で呼ばれる
    void countDown()
    {
        remainTime -= InterruptTime;
        if (remainTime < 0)
        {
            if (++idx > 5)
            {
                remainTime = pulse[idx];
                isOn != isOn;
            }
            else
            {
                //1セット終了
                remainTime = 0;
            }
        }
    }

private:
    volatile bool isOn;
    volatile bool idx;
    volatile int pulse[6];
    volatile int remainTime;
};
haptic ptn160 = haptic(3125);

void setup()
{
    Nefry.enableSW();

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    // Create semaphore to inform us when the timer has fired
    timerSemaphore = xSemaphoreCreateBinary();

    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).
    timer = timerBegin(0, 80, true);

    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer, &onTimer, true);

    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, InterruptTime, true);

    // Start an alarm
    timerAlarmEnable(timer);
}

void loop()
{
    //割り込み処理
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
        ptn160.countDown();
    }

    //パルス(160Hz)
    interval<LOOPTIME_PULSE_160>::run([] {
        ptn160.start();
    });

    //データ・グラフ更新
    interval<LOOPTIME_GRAPH>::run([] {
        //描画する
        NefryDisplay.clear();
        NefryDisplay.drawString(10, 10, String(ptn160.getisOn()));
        NefryDisplay.display();
    });
}
