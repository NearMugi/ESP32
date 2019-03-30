#ifndef GOOGLE_CLOUDFUNCTIONS_H
#define GOOGLE_CLOUDFUNCTIONS_H
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define NEFRY_GCP_PROJECT 8

class googleCloudFunctions {
  public:
    const char* host;
    const int httpsPort = 443;
    String basePostHeader;
    
    void InitAPI() {
      Nefry.setStoreTitleStr("GCP Project", NEFRY_GCP_PROJECT);
      String _tmp = Nefry.getStoreStr(NEFRY_GCP_PROJECT) + ".cloudfunctions.net";
      char* _tmpC;
      _tmp.toCharArray(_tmpC, _tmp.length() + 1);
      host = (const char*)_tmpC;
      
      //リクエストするときのヘッダーを設定する
      basePostHeader = "";
      basePostHeader += ("POST @function HTTP/1.1\r\n");
      basePostHeader += ("Host: " + String(host) + ":" + String(httpsPort) + "\r\n");
      basePostHeader += ("Connection: close\r\n");
      basePostHeader += ("Content-Type: application/json");
    }
    
    //RuntimeConfigからデータを取得する
    String getRuntimeConfig(String _configName) {
      String function = "/GetRuntimeConfig";
      
      String postData = "";
      postData += "{""list"" : [""" + _configName + """]}";

      String postHeader = basePostHeader;
      postHeader.replace("@function", function);
      
      String ret = postRequest(host, postHeader, postData);
      Serial.println(ret);
      return "";
    }


  private:
    //サーバーにデータをポストする
    String postRequest(const char* server, String header, String data) {

      String result = "";

      // Use WiFiClientSecure class to create TLS connection
      WiFiClientSecure client;
      Serial.print("Connecting to: "); Serial.println(server);

      if (!client.connect(server, httpsPort)) {
        Serial.println("connection failed");
        return result;
      }
      Serial.println("certificate matches");

      Serial.print("post: "); Serial.println(header + data);
      client.print(header + data);

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
