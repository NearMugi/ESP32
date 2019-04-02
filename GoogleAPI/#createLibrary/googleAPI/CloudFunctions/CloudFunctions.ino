//CloudFunctionsを使う。

//callCloudFunctions
//CloudFunctionsにPOSTリクエストする
//引数：関数名,POSTデータ
//返値：指定した関数からの返値

//getRuntimeConfig
//RuntimeConfigのデータを取得する
//引数：POSTするJSONデータ {"list" : ["CONFIGNAME1","CONFIGNAME2"]}
//返値：[{"lbl":"CONFIGNAME","data":{"変数名1":"値","変数名2":"値"}}]

#include <Nefry.h>
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;


#include <ArduinoJson.h>


void setup() {
  cfs.InitAPI();

  //RuntimeConfigのデータを取得する
  Serial.println("\n++++++++++++++++++++++\n[RuntimeConfig]\n++++++++++++++++++++++\n");
  String ret = cfs.getRuntimeConfig("googleAPI");

  //取得したjsonデータから欲しい情報を取得する
  String GPSApiKey = cfs.getJsonValue(ret, "GPSApiKey");
  
  //GPSMap_getMapBinaryを使う
  Serial.println("\n++++++++++++++++++++++\n[GPSMap_getMapBinary]\n++++++++++++++++++++++\n");
  String _axis = "35.68035,139.7673229";
  String postData = "";
  postData += "{\"key\" : \"" + GPSApiKey + "\",";
  postData += "\"axis\" : \"" + _axis + "\",";
  postData += "\"color\" : \"true\"";
  postData += "}";
  Serial.println(postData);

  ret = cfs.callCloudFunctions("GPSMap_getMapBinary", postData);
  Serial.println(ret);

}

void loop() {
}
