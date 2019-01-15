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

void setup() {
  Nefry.setProgramName("Post GoogleDrive");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);

  Nefry.enableSW();

  api.InitAPI();
}

void loop() {
  if (Nefry.readSW()) {

    String fn = "myObject";
    String txtData = "HOGEhoge";
    String comment = "From NefryBT";
    NefryDisplay.clear();

    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, F("File:"));
    NefryDisplay.drawString(0, 50, fn);
    NefryDisplay.drawString(10, 0, F("Data:"));
    NefryDisplay.drawString(10, 50, txtData);
    NefryDisplay.drawString(20, 0, F("Comment:"));
    NefryDisplay.drawString(20, 50, comment);

    String msg = api.postDrive_Text(fn, txtData, comment);
    NefryDisplay.drawString(30, 0, msg);
  }
}
