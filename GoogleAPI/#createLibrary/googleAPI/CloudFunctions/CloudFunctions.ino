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

  String retBytes = cfs.callCloudFunctions("GPSMap_getMapBinary", postData);
  uint8_t imgBytes[retBytes.length() + 1];
  retBytes.getBytes(imgBytes, sizeof(imgBytes));

  delay(1000);

  Serial.print("retBytes Size: ");
  Serial.println(retBytes.length());
  Serial.print("imgBytes Size: ");
  Serial.println(sizeof(imgBytes));
  for(int i=0; i< sizeof(imgBytes)-1; i++){
    Serial.print(String(imgBytes[i], HEX)); 
    Serial.print(" "); 
  }

}

void loop() {
}
