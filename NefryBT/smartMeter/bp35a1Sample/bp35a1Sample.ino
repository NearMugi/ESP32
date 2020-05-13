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

// Nefry Environment data

#define smartMeterIdx 7
#define smartMeterTag "Smart Meter ID,Password"

// ループ周期(ms)

// 即時電力値・電流値・累積電力値を取得
#define LOOPTIME_GET_EP_VALUE 60 * 1000
// 接続確認
#define LOOPTIME_CHECK_CONNECT 30 * 1000

void setup()
{
    Nefry.setProgramName("Access SmartMeter");
    Nefry.setStoreTitle(smartMeterTag, smartMeterIdx);

    // シリアル通信するポート(RX:D2=23, TX:D3=19);
    int PIN_RX = 23;
    int PIN_TX = 19;
    String tmp = Nefry.getStoreStr(smartMeterIdx);
    int tmpIdx = tmp.indexOf(",");
    String serviceID = tmp.substring(0, tmpIdx);
    String servicePW = tmp.substring(tmpIdx + 1);
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
        // 瞬時電力(A)
        Serial.println(bp.epA);
        // 電流値(kW)
        Serial.println(bp.epkW);
        // 累積電力値(kwh)
        Serial.println(bp.totalkWh);
        // 累積電力値の対象時間
        Serial.println(bp.date);
    });
}