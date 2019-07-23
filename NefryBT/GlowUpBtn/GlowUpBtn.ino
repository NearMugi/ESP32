//曲とモーターを稼働させる。

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

//モーター(トリガーを渡せば可動する)
#define PIN_MOTOR 22 //D0

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_UPDATE 100000
#define LOOPTIME_DISP 10000

#define SOUND_CNT 5 //曲数
int wavNo;
bool isLoop;

float nextMotCnt; //次可動させるタイミング
int MotCnt;       //シーケンス開始からのカウント
int idxMot;       //配列のインデックス
#define MAX_ON 11 //1つの曲の中でONにする最大回数

float size = 1000000 / LOOPTIME_UPDATE;
float ofs = 0.1;

//シーケンス -1はシーケンス終了の印
//ここの数値は1s単位、読み込んで設定する際に実際の数値に修正する
float motSeq[SOUND_CNT][MAX_ON] = {
    {
        0.64,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
    }, //Mov1
    {
        0.20,
        1.50,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
    }, //Mov2
    {
        5.30,
        8.32,
        10.30,
        13.04,
        15.46,
        18.26,
        21.10,
        -1,
        -1,
        -1,
        -1,
    }, //Mov3
    {
        0.52,
        2.46,
        4.30,
        6.32,
        8.20,
        10.22,
        11.54,
        13.48,
        15.44,
        -1,
        -1,
    }, //Mov4
    {
        2.38,
        3.54,
        6.42,
        8.32,
        12.06,
        13.44,
        15.48,
        -1,
        -1,
        -1,
        -1,
    }, //Mov5
};

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
    wavNo = 0;
    isLoop = false;
    pinMode(PIN_MOTOR, OUTPUT);
    digitalWrite(PIN_MOTOR, HIGH);
    MotCnt = -1;
    idxMot = -1;
    nextMotCnt = -1;
}

float setNextMotCnt()
{
    float _tmp = motSeq[wavNo - 1][idxMot];
    if (_tmp < 0)
        return -1;
    _tmp = _tmp * size - ofs;
    if (_tmp < 0)
        _tmp = 0;
    return _tmp;
}

void loop()
{
    //debug
    if (Nefry.readSW())
    {
        wavNo = wavNo % SOUND_CNT + 1;
        //wavNo = 6;

        myDFPlayer.play(wavNo);
        MotCnt = 0;
        idxMot = 0;
        nextMotCnt = setNextMotCnt();

        isLoop = false;
        if (wavNo == 1 || wavNo == 2)
        {
            isLoop = true;
        }
    }

    interval<LOOPTIME_UPDATE>::run([] {
        digitalWrite(PIN_MOTOR, HIGH);
        if (nextMotCnt >= 0)
        {
            MotCnt++;
            if (nextMotCnt < MotCnt)
            {
                digitalWrite(PIN_MOTOR, LOW);
                if (++idxMot >= MAX_ON)
                {
                    idxMot = MAX_ON;
                    nextMotCnt = -1;
                }
                else
                {
                    nextMotCnt = setNextMotCnt();
                }
            }
        }
        else
        {
            //終了まで読み込んだあと、ループ指定があればもう一度再生する。
            if (isLoop)
            {
                myDFPlayer.play(wavNo);
                MotCnt = 0;
                idxMot = 0;
                nextMotCnt = setNextMotCnt();
            }
        }
    });

    //ディスプレイ
    interval<LOOPTIME_DISP>::run([] {
        NefryDisplay.clear();
        NefryDisplay.setFont(ArialMT_Plain_10);
        NefryDisplay.drawString(10, 5, "WavNo : " + String(wavNo));
        NefryDisplay.drawString(10, 20, "now(s) : " + String((LOOPTIME_UPDATE * MotCnt) / (float)1000000));
        NefryDisplay.drawString(10, 35, "next   : " + String(nextMotCnt));
        NefryDisplay.drawString(10, 48, "now    : " + String(MotCnt));
        NefryDisplay.display();
    });
}
