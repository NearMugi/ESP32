#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include "intervalMs.h"
#include "bp35a1.h"

void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

bp35a1 bp;

// ループ周期(ms)

// 即時電力値・電流値・累積電力値を取得
#define LOOPTIME_GET_EP_VALUE 60 * 1000
// 接続確認
#define LOOPTIME_CHECK_CONNECT 30 * 1000

void setup()
{
    // シリアル通信するポート(RX:D2=23, TX:D3=19);
    int PIN_RX = 23;
    int PIN_TX = 19;
    String serviceID = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    String servicePW = "XXXXXXXXXXXX";
    bp.init(PIN_RX, PIN_TX, serviceID, servicePW);
}

void loop()
{
    bp.connect();

    interval<LOOPTIME_CHECK_CONNECT>::run([] {
        bp.chkConnect();
    });

    interval<LOOPTIME_GET_EP_VALUE>::run([] {
        bp.getEPValue();
    });
}