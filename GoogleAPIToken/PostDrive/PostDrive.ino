#include <Nefry.h>

#include "googleAPI.h"
googleAPI api;

#define NEFRY_DATASTORE_REFRESH_TOKEN 5
#define NEFRY_DATASTORE_CLIENT_ID 6
#define NEFRY_DATASTORE_CLIENT_SECRET 7

String refresh_token = "";
String client_id = "";
String client_secret = "";
String accessToken = "";

const char* host = "www.googleapis.com";
String token_uri = "/upload/drive/v3/files?uploadType=multipart";
const int httpsPort = 443;

void postDrive() {
  Serial.println(F("[Start Post to Drive]"));
  
  static uint8_t postData[] = "HOGEhoge";
  uint8_t DataSize = sizeof(postData) - 1;

  String fileName = "myObject";
  
  Serial.println(DataSize);
  String start_request = "";
  start_request = start_request +
                  "\r\n--foo_bar_baz\r\n" +
                  "Content-Type: application/json; charset=UTF-8\r\n" +
                  "\r\n{\r\n" +
                  "\t\"name\": \"" + fileName + "\"\r\n" +
                  "}\r\n\r\n" +
                  "--foo_bar_baz\r\n" +
                  "Content-Type: text/plain\r\n\r\n";

  String end_request = "\r\n--foo_bar_baz--\r\n";
  uint16_t full_length;
  full_length = start_request.length() + DataSize + end_request.length();



  String postHeader = "";
  postHeader += ("POST " + token_uri + " HTTP/1.1\r\n");
  postHeader += ("Host: " + String(host) + ":" + String(httpsPort) + "\r\n");
  postHeader += ("Connection: close\r\n");
  postHeader += ("Content-Type: multipart/related; boundary=foo_bar_baz\r\n");
  postHeader += ("Content-Length: ");
  postHeader += (full_length);
  postHeader += ("\r\n");
  postHeader += ("Authorization: Bearer " + accessToken + "\r\n");
  postHeader += ("\r\n");

  String result = "";

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("Connecting to: "); Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return ;
  }

  Serial.println("[Header]"); Serial.println(postHeader);
  Serial.println("[start_request]"); Serial.println(start_request);
  Serial.println("[end_request]"); Serial.println(end_request);
  client.print(postHeader + start_request);

  client.write(&postData[0] , DataSize);

  client.println(end_request);

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
    } else {
      Serial.println(F("[WARNING] Response Data is Nothing"));
    }

    String line = client.readStringUntil('\r');
    Serial.println(line);
    result += line;
  }

  Serial.println("closing connection");
}

void setup() {
  Nefry.setProgramName("Post GoogleDrive");

  Nefry.setStoreTitleStr("Refresh Token", NEFRY_DATASTORE_REFRESH_TOKEN);
  Nefry.setStoreTitleStr("Client ID", NEFRY_DATASTORE_CLIENT_ID);
  Nefry.setStoreTitleStr("Client Secret", NEFRY_DATASTORE_CLIENT_SECRET);

  refresh_token = Nefry.getStoreStr(NEFRY_DATASTORE_REFRESH_TOKEN);
  client_id = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_ID);
  client_secret = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_SECRET);

  delay(5000);

  accessToken = api.GetAccessToken(refresh_token, client_id, client_secret);
  Serial.println(accessToken);

  postDrive();
}

void loop() {
  // put your main code here, to run repeatedly:

}
