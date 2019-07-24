//曲とモーターを稼働させる。

#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include "BLE_Central.h"

//[コンストラクタ]CentralName, PeripheralName, ServiceUUID, ReadCharUUID, WriteCharUUID
BLE_Central ble(
    "NefryBLE",
    "AndroidBLE",
    "da61480e-4ad8-11e9-8646-d663bd873d93",
    "7b15e552-4ad9-11e9-8646-d663bd873d93",
    "e1d67b1c-4ae3-11e9-8646-d663bd873d93");

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

//BLE
String readValue;
String writeValue;
byte tmpWrite;

#define SOUND_CNT 5 //曲数
int wavNoBef;
int wavNo;
bool isLoop;

unsigned long t;
int intTime;
int nextMotCnt; //次可動させるタイミング
int MotCnt;       //シーケンス開始からのカウント
int idxMot;       //配列のインデックス
#define MAX_ON 9 //1つの曲の中でONにする最大回数

int ofs = 100;

//シーケンス -1はシーケンス終了の印
//ms単位
int motSeq[SOUND_CNT][MAX_ON] = {
    {
        640,
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
        200,
        1500,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
    }, //Mov2
    {
        5300,
        8320,
        10300,
        13040,
        15460,
        18260,
        21100,
        -1,
        -1,
    }, //Mov3
    {
        520,
        2460,
        4300,
        6320,
        8200,
        10220,
        11540,
        13480,
        15440,
    }, //Mov4
    {
        2380,
        3540,
        6420,
        8320,
        12060,
        13440,
        15480,
        -1,
        -1,
    }, //Mov5
};

bool initDFPlayer()
{
    mySoftwareSerial.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX); // (speed, type, RX:D2=23, TX:D3=19);

    if (!myDFPlayer.begin(mySoftwareSerial))
    { //Use softwareSerial to communicate with mp3.
        return false;
    }

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

    ble.init();
    readValue = "";
    writeValue = "";
    tmpWrite = 0;

    isActive = initDFPlayer();
    wavNo = 0;
    wavNoBef = 0;
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
    _tmp = _tmp  - ofs;
    if (_tmp < 0)
        _tmp = 0;
    return _tmp;
}

void setWav(int _no)
{
    if (!isActive)
        return;

    if (_no <= 0 || _no > SOUND_CNT)
    {
        return;
    }

    wavNo = _no;
    wavNoBef = wavNo;

    //AndroidにWavNoを伝える
    writeValue = String(wavNo);
    ble.setWriteValue(writeValue);
    //タイミング調整
    delay(1000);
    //音再生
    myDFPlayer.play(wavNo);
    t = millis();

    MotCnt = 0;
    idxMot = 0;
    nextMotCnt = setNextMotCnt();

    isLoop = false;
    if (wavNo == 1 || wavNo == 2)
    {
        isLoop = true;
    }
}

void loop()
{
    ble.loop();
    ble.update();
    if (ble.getIsConnect())
    {
        //read
        readValue = ble.getReadValue();
        readValue.toLowerCase();
        if(readValue.toInt() != wavNoBef){
            setWav(readValue.toInt());
        }
    }

    intTime = millis() - t;
    t = millis();
    digitalWrite(PIN_MOTOR, HIGH);
    if (nextMotCnt >= 0)
    {
        MotCnt += intTime;
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
            setWav(wavNo);
        }
    }

    NefryDisplay.clear();
    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(10, 0, "WavNo : " + String(wavNo));
    NefryDisplay.drawString(10, 12, "intTime: " + String(intTime));
    NefryDisplay.drawString(10, 25, "next   : " + String(nextMotCnt));
    NefryDisplay.drawString(10, 37, "now    : " + String(MotCnt));
    NefryDisplay.drawString(10, 50, ble.getNowStatus_st());
    NefryDisplay.display();
}
