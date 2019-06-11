
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
#define PIN_RX 23
#define PIN_TX 19

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_PULSE_160 200000

class haptic
{
public:
    haptic(int no)
    {
        wavNo = no;
    }
    void play()
    {
        myDFPlayer.play(wavNo);
    }

private:
    int wavNo;
};
haptic ptn160 = haptic(1);

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
    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    NefryDisplay.clear();
    NefryDisplay.display();

    isActive = initDFPlayer();
}

void loop()
{
    interval<LOOPTIME_PULSE_160>::run([] {
        if (isActive)
            ptn160.play();
    });
}
