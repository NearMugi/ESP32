#ifndef GOOGLE_CLOUDFUNCTIONS_H
#define GOOGLE_CLOUDFUNCTIONS_H
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define NEFRY_GCP_PROJECT 8

class googleCloudFunctions {
  public:
    String basePostHeader;

    void InitAPI() {
      Nefry.setStoreTitleStr("GCP Project", NEFRY_GCP_PROJECT);
      //リクエストするときのヘッダーを設定する
      basePostHeader = "";
      basePostHeader += ("POST @function HTTP/1.1\r\n");
      basePostHeader += ("Host: @host:@httpsPort\r\n");
      basePostHeader += ("Connection: close\r\n");
      basePostHeader += ("Content-Type: application/json;charset=utf-8\r\n");
    }

    //RuntimeConfigからデータを取得する
    String getRuntimeConfig(String _configName) {
      String postData = "{\"list\" : [\"" + _configName + "\"]}";
      return callCloudFunctions("GetRuntimeConfig", postData);
    }

    //関数名とPOSTリクエストを指定して実行、レスポンスを返す
    String callCloudFunctions(String _funcName, String _postData) {
      String _tmp = Nefry.getStoreStr(NEFRY_GCP_PROJECT);
      const char* host = _tmp.c_str();
      const int httpsPort = 443;
      String function = "/" + _funcName;
      String postData = _postData;

      String postHeader = basePostHeader;
      postHeader.replace("@function", function);
      postHeader.replace("@host", String(host));
      postHeader.replace("@httpsPort", String(httpsPort));
      postHeader = postHeader + "Content-Length: " + postData.length() + "\r\n" + "\r\n" + postData + "\r\n";

      String ret = postRequest(host, httpsPort, postHeader);
      return ret;
    }

    //jsonデータから欲しい情報を取得する
    String getJsonValue(String _target, String _member) {
      const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(1);
      StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
      char json[_target.length() + 1];
      _target.toCharArray(json, sizeof(json));
      JsonObject& root = jsonBuffer.parseObject(json);
      const char* ret = root[_member];
      return ret;
    }

  private:
    //サーバーにデータをポストする
    String postRequest(const char* server, int port, String header) {

      String result = "";

      // Use WiFiClientSecure class to create TLS connection
      WiFiClientSecure client;
      Serial.print("Connecting to: "); Serial.println(server);

      if (!client.connect(server, port)) {
        Serial.println("connection failed");
        return result;
      }
      Serial.println("certificate matches");

      Serial.println("[post]");
      Serial.println(header);
      client.print(header);

      Serial.println("Receiving response");
      if (client.connected()) {
        if (client.find("HTTP/1.1 ")) {
          String status_code = client.readStringUntil('\r');
          Serial.print("Status code: "); Serial.println(status_code);
          if (status_code != "200 OK") {
            Serial.println("There was an error");
          }
        }
        if (client.find("\r\n\r\n")) {
          Serial.println(F("[Read Data]"));
        }
        String line = client.readStringUntil('\r');
        Serial.println(line);
        result += line;
      }

      Serial.println("closing connection");
      return result;
    }

};
#endif
