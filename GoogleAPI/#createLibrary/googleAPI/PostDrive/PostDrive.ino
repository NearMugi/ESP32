#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

#include "googleAPI.h"
googleAPI api;
// Data Store
// 5:Refresh Token
// 6:Client ID
// 7:Client Secret
// 8:Parent Folder

//date
#include <time.h>
#define JST     3600*9

void setup() {
  Nefry.setProgramName("Post GoogleDrive");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  Nefry.enableSW();

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");


  NefryDisplay.clear();
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, F("[Post Text File]"));
  NefryDisplay.drawString(0, 10, F("Init API Setting..."));
  NefryDisplay.display();

  api.InitAPI();

  NefryDisplay.drawString(0, 20, F("Success in Setting!"));
  NefryDisplay.drawString(0, 30, F("Waiting Push Btn..."));
  NefryDisplay.display();
  Nefry.ndelay(10);


}

void loop() {
  if (Nefry.readSW()) {

    //日付を取得する
    time_t  t = time(NULL);
    struct tm *tm;
    tm = localtime(&t);
    char _fn[20] = "";
    sprintf(_fn, "%04d%02d%02d_%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    String fn = String(_fn);
    String txtData = "HOGEhoge";
    String comment = "From NefryBT";

    NefryDisplay.clear();
    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, F("File:"));
    NefryDisplay.drawString(20, 0, fn);
    NefryDisplay.drawString(0, 10, F("Data:"));
    NefryDisplay.drawString(30, 10, txtData);
    NefryDisplay.drawString(0, 20, F("Comment:"));
    NefryDisplay.drawString(50, 20, comment);
    NefryDisplay.display();

    String msg = api.postDrive_Text(fn, txtData, comment);

    NefryDisplay.drawString(0, 30, msg);
    NefryDisplay.display();
  }
}
