#ifndef GOOGLE_CLOUDFUNCTIONS_H
#define GOOGLE_CLOUDFUNCTIONS_H
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

class googleCloudFunctions
{
public:
  String basePostHeader;

  //リクエストするときのヘッダーを設定する
  void InitAPI()
  {
    basePostHeader = "";
    basePostHeader += ("POST @function HTTP/1.1\r\n");
    basePostHeader += ("Host: @host:@httpsPort\r\n");
    basePostHeader += ("Connection: close\r\n");
    basePostHeader += ("Content-Type: application/json;charset=utf-8\r\n");
  }

  //RuntimeConfigからデータを取得する
  String getRuntimeConfig(String _host, String _configName)
  {
    String postData = "{\"list\" : [\"" + _configName + "\"]}";
    return callCloudFunctions_Json(_host, "GetRuntimeConfig", postData);
  }

  //関数名とPOSTリクエストを指定して実行、レスポンス(JSON)を返す
  String callCloudFunctions_Json(String _host, String _funcName, String _postData)
  {
    char *res;
    res = (char *)malloc(2000 * sizeof(char));
    int len = postCloudFunctions(_host, _funcName, _postData, res);

    String ret = "";
    for (int i = 0; i < len; i++)
    {
      ret += res[i];
    }
    //最初と最後の"[]"を外す
    ret.setCharAt(0, ' ');
    ret.setCharAt(ret.length() - 1, ' ');
    ret.trim();
#ifdef DEBUG
    Serial.print(F("[Get Data] Size : "));
    Serial.println(ret.length());
    Serial.println(ret);
#endif
    free(res);
    return ret;
  }

  //関数名とPOSTリクエストを指定して実行、レスポンス(String)を返す
  String callCloudFunctions_String(String _host, String _funcName, String _postData)
  {
    char *res;
    res = (char *)malloc(6000 * sizeof(char));
    int len = postCloudFunctions(_host, _funcName, _postData, res);

    //Serial.println(len);
    String ret = "";
    for (int i = 0; i < len; i++)
    {
      ret += res[i];
      //Serial.print(String(res[i], HEX));
      //Serial.print(" ");
    }
    //Serial.println("");

    free(res);
    return ret;
  }

  //jsonデータから欲しい情報を取得する
  String getJsonValue(String _target, String _member)
  {
    StaticJsonDocument<700> doc;
    char json[_target.length() + 1];
    int size = sizeof(json);
    _target.toCharArray(json, size);

    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return "";
    }
    const char *ret = doc[_member];
    return ret;
  }

private:
  //CloudFunctionsにポストする
  //取得したサイズを返す
  int postCloudFunctions(String _host, String _funcName, String _postData, char *res)
  {
    const char *host = _host.c_str();
    const int httpsPort = 443;
    String function = "/" + _funcName;
    String postData = _postData;

    String postHeader = basePostHeader;
    postHeader.replace("@function", function);
    postHeader.replace("@host", String(host));
    postHeader.replace("@httpsPort", String(httpsPort));
    postHeader = postHeader + "Content-Length: " + postData.length() + "\r\n" + "\r\n" + postData + "\r\n";

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
#ifdef DEBUG
    Serial.print(F("Connecting to: "));
    Serial.println(host);
#endif

    if (!client.connect(host, httpsPort))
    {
      Serial.println(F("connection failed"));
      return 0;
    }
#ifdef DEBUG
    Serial.println(F("certificate matches"));
    Serial.println(F("[post]"));
    Serial.println(postHeader);
#endif

    client.print(postHeader);
    delay(10);

    while (client.connected())
    {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
#ifdef DEBUG
        Serial.println(F("headers received"));
#endif
        break;
      }
    }

    delay(1000);
    int len = 0;
    while (client.available())
    {
      char c = client.read();
      *(res++) = c;
      len++;
      //Serial.write(c);
      //Serial.print(String(c,HEX));
      //Serial.print(" ");
    }
#ifdef DEBUG
    Serial.println(F("\nclient stop"));
#endif
    client.stop();

    return len;
  }
};
#endif
