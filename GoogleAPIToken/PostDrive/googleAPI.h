#ifndef GOOGLEAPI_H
#define GOOGLEAPI_H
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

class googleAPI {
    const char* host = "www.googleapis.com";
    String token_uri = "/oauth2/v4/token";
    const int httpsPort = 443;

  public:
    String GetAccessToken(String refresh_token, String client_id, String client_secret) {
      String token = "";
      String postData = "";
      postData += "refresh_token=" + refresh_token;
      postData += "&client_id=" + client_id;
      postData += "&client_secret=" + client_secret;
      postData += "&grant_type=" + String("refresh_token");

      String postHeader = "";
      postHeader += ("POST " + token_uri + " HTTP/1.1\r\n");
      postHeader += ("Host: " + String(host) + ":" + String(httpsPort) + "\r\n");
      postHeader += ("Connection: close\r\n");
      postHeader += ("Content-Type: application/x-www-form-urlencoded\r\n");
      postHeader += ("Content-Length: ");
      postHeader += (postData.length());
      postHeader += ("\r\n\r\n");

      String result = postRequest(host, postHeader, postData);

      //取得したjsonデータからAccessTokenを取得する
      const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(1);
      StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
      char json[result.length() + 1];
      result.toCharArray(json, sizeof(json));
      JsonObject& root = jsonBuffer.parseObject(json);
      
      const char* tmp = root["access_token"];
      token = tmp;
      return token;
    }

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

      Serial.println("request sent");
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
