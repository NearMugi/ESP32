//CloudFunctionsを使う。

//+ callCloudFunctions_String
//CloudFunctionsにPOSTリクエストする
//引数：ホスト,関数名,POSTデータ
//返値：指定した関数からの返値(String)

//+ callCloudFunctions_Json
//基本callCloudFunctions_Stringと同じ。jsonデータを返す

//+ getRuntimeConfig
//RuntimeConfigのデータを取得する
//引数：list
//返値：{"lbl":"CONFIGNAME","data":{"変数名1":"値","変数名2":"値"}}

//+ getJsonValue
//jsonからデータを取得する
//引数：json, ターゲット
//返値：ターゲットの値

#include <Nefry.h>
#include <ArduinoJson.h>

//CloudFunctions
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;

//https
#define NEFRY_GCP_PROJECT 8
String host = ""; //"[プロジェクト名].cloudfunctions.net"をNefryのデータストアから取得する
String apikey = "";

//RuntimeConfigのデータを取得する
String getRuntimeConfigData(String _list, String _name)
{
  //RuntimeConfigのデータを取得する
  String ret = cfs.getRuntimeConfig(host, _list);

  //取得したjsonデータから欲しい情報を取得する
  return cfs.getJsonValue(ret, _name);
}

void setup()
{
  //https
  Nefry.setStoreTitleStr("GCP Project", NEFRY_GCP_PROJECT);
  host = Nefry.getStoreStr(NEFRY_GCP_PROJECT);

  //CloudFunctions
  cfs.InitAPI();
  apikey = getRuntimeConfigData("googleAPI", "GPSApiKey");

  //CloudFunctionsを使う例(GPSMap_getMapBinary)
  String _axis = "35.68035,139.7673229";
  String postData = "";
  postData += "{\"key\" : \"" + apikey + "\",";
  postData += "\"axis\" : \"" + _axis + "\",";
  postData += "\"color\" : \"true\",";
  postData += "\"trim\" : \"1\"";
  postData += "}";
  Serial.println(postData);

  String retImage = cfs.callCloudFunctions_String(host, "GPSMap_getMapBinary", postData);
  int len = retImage.length();
  Serial.print(F("[Get Data] Size : "));
  Serial.println(len);
  Serial.println(retImage);
  uint8_t bytes[len];
  for (int i = 0; i < len; i++)
  {
    bytes[i] = retImage[i];
    Serial.print(String(bytes[i], HEX));
    Serial.print(" ");
  }
  Serial.println("");
}

void loop()
{
}
