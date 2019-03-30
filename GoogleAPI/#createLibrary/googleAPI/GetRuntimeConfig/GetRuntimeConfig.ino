//RuntimeConfigのデータを取得する
//POSTするJSONデータ {"list" : ["CONFIGNAME"]}
//返値 [{"lbl":"CONFIGNAME","data":{"変数名1":"値","変数名2":"値"}}]

#include <Nefry.h>
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;

void setup() {
  cfs.InitAPI();
  cfs.getRuntimeConfig("googleAPI");
}

void loop() {
}
