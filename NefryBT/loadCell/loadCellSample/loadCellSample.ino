// ロードセルを使って重さを測定する
// サンプルコード http://akizukidenshi.com/catalog/g/gK-12370/
#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include "interval.h"
#include "loadCell.h"

void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

//ループ周期(us)
#define LOOPTIME_READ 10000
#define LOOPTIME_DISP 10000

// LoadCell
loadCell lc;
float readData;
#define pin_dout 13    //D7
#define pin_slk 14     //D8
#define OUT_VOL 0.001f //定格出力 [V]
#define LOAD 2000.0f   //定格容量 [g]

void setup()
{
    Nefry.enableSW();

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    lc.init(pin_dout, pin_slk, OUT_VOL, LOAD);
}

void loop()
{
    // Read LoadCell
    interval<LOOPTIME_READ>::run([] {
        readData = lc.getData();
    });

    // ディスプレイ
    interval<LOOPTIME_DISP>::run([] {
        NefryDisplay.clear();
        NefryDisplay.setFont(ArialMT_Plain_16);
        char s[20];
        dtostrf(readData, 5, 3, s);
        NefryDisplay.drawString(10, 10, "Weight :" + String(s));
        NefryDisplay.display();
    });
}