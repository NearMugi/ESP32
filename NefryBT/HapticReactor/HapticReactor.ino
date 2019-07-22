
#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
    Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

#include "DFRobotDFPlayerMini.h"

HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;
bool isActive;
#define PIN_RX 14 //D8
#define PIN_TX 13 //D7

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_PULSE 10000
#define LOOPTIME_DISP 10000
#define LOOPTIME_PLAY 5000000

int SOUND_CNT = 6; //曲数
//debug
int cnt = 1; //ループ回数

class haptic
{
public:
    int wavNo;
    int lpCnt;
    haptic()
    {
        wavNo = 1;
        lpCnt = 0;
    }
    void play(int lp)
    {
        lpCnt = lp;
        myDFPlayer.play(wavNo);
    }
    void update()
    {
        if (lpCnt <= 0)
            return;

        if (!myDFPlayer.available())
            return;

        //再生中なら何もしない
        if (myDFPlayer.readType() != DFPlayerPlayFinished)
            return;

        if (--lpCnt <= 0)
        {
            lpCnt = 0;
        }
        else
        {
            myDFPlayer.play(wavNo);
        }
    }
};
haptic hap = haptic();

bool initDFPlayer()
{
    mySoftwareSerial.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX); // (speed, type, RX:D2=23, TX:D3=19);

    if (!myDFPlayer.begin(mySoftwareSerial))
    { //Use softwareSerial to communicate with mp3.

        Serial.println(F("Unable to begin..."));
        return false;
    }
    Serial.println(F("DFPlayer Mini online."));

    myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

    //----Set volume----
    myDFPlayer.volume(30);   //Set volume value (0~30).
    myDFPlayer.volumeUp();   //Volume Up
    myDFPlayer.volumeDown(); //Volume Down

    //----Set different EQ----
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);

    //----Set device we use SD as default----
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

    return true;
}

void setup()
{
    Nefry.enableSW();

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    isActive = initDFPlayer();
}

void loop()
{
    //debug
    if (Nefry.readSW())
    {
        hap.wavNo = hap.wavNo % SOUND_CNT + 1;
        cnt = cnt % 10 + 1;
    }

    interval<LOOPTIME_PLAY>::run([] {
        if (isActive)
            hap.play(cnt);
    });

    interval<LOOPTIME_PULSE>::run([] {
        if (isActive)
            hap.update();
    });

    //ディスプレイ
    interval<LOOPTIME_DISP>::run([] {
        NefryDisplay.clear();
        NefryDisplay.setFont(ArialMT_Plain_16);
        NefryDisplay.drawString(10, 10, "WavNo :" + String(hap.wavNo));
        NefryDisplay.drawString(10, 30, "Loop(" + String(cnt) + ")  :" + String(hap.lpCnt));
        NefryDisplay.drawString(10, 50, "Type  :" + String(myDFPlayer.readType()));
        NefryDisplay.display();
    });
}
