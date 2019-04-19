#include <Nefry.h>

#include "googleAPI.h"
googleAPI api;
// Data Store
// 5:Refresh Token
// 6:Client ID
// 7:Client Secret
// 8:Parent Folder

void setup() {
  Nefry.setProgramName("Post GoogleDrive");
  
  api.InitAPI();
  api.postDrive_Text("myObject","HOGEhoge", "From NefryBT");
}

void loop() {
  
}
