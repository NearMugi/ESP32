#include <Nefry.h>
#include <NefryDisplay.h>
#include <HTTPClient.h>

#define NEFRY_DATASTORE_REFRESH_TOKEN 5
#define NEFRY_DATASTORE_CLIENT_ID 6
#define NEFRY_DATASTORE_CLIENT_SECRET 7

#define BODY_SIZE 206
char body[BODY_SIZE];

void setupAccessToken() {
  Nefry.setStoreTitleStr("Refresh Token", NEFRY_DATASTORE_REFRESH_TOKEN);
  Nefry.setStoreTitleStr("Client ID", NEFRY_DATASTORE_CLIENT_ID);
  Nefry.setStoreTitleStr("Client Secret", NEFRY_DATASTORE_CLIENT_SECRET);

  String _tmp = "";
  _tmp += F("refresh_token=");
  _tmp += Nefry.getStoreStr(NEFRY_DATASTORE_REFRESH_TOKEN);
  _tmp += F("&");

  _tmp += F("client_id=");
  _tmp += Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_ID);
  _tmp += F("&");

  _tmp += F("client_secret=");
  _tmp += Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_SECRET);
  _tmp += F("&");
  _tmp += F("grant_type=refresh_token");
  _tmp.toCharArray(body, BODY_SIZE + 1);
}

String getAccessToken() {
  String ret_Token = "";
  Serial.println(F("++++++++++++++++++++\n[Get AccessToken]\n++++++++++++++++++++"));

  HTTPClient http;
  http.begin("https://www.googleapis.com/oauth2/v4/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(body);
  Serial.println(httpCode);
  String result = "";
  if (httpCode < 0) {
    result = http.errorToString(httpCode);
  } else {
    Serial.println(http.getSize());
    result = http.getString();
  }

  http.end();
  Serial.println(result);

  return ret_Token;
}

void setup() {
  Nefry.setProgramName("Get GoogleAPI AccessToken");

  Nefry.enableSW();

  setupAccessToken();

}

void loop() {
  if (Nefry.readSW()) {
    delay(1000);
    getAccessToken();
  }

}
