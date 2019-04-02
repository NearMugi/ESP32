//RuntimeConfigのデータを取得する
//POSTするJSONデータ {"list" : ["CONFIGNAME1","CONFIGNAME2"]}
//返値 [{"lbl":"CONFIGNAME","data":{"変数名1":"値","変数名2":"値"}}]

#include <Nefry.h>
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;

void setup() {
  cfs.InitAPI();
  String ret = cfs.getRuntimeConfig("googleAPI");

  //取得したjsonデータから欲しい情報を取得する
  String retData = cfs.getJsonValue(ret,"data");
  String GPSApiKey = cfs.getJsonValue(retData,"GPSApiKey");
  
  Serial.println(retData);
  Serial.println(GPSApiKey);

}

void loop() {
}
