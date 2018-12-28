#include <Nefry.h>
#include <NefryDisplay.h>
#include <HTTPClient.h>

#define NEFRY_DATASTORE_REFRESH_TOKEN 5
#define NEFRY_DATASTORE_CLIENT_ID 6
#define NEFRY_DATASTORE_CLIENT_SECRET 7
String getAccessToken(String _refreshToken, String _clientID, String  _clientSecret) {
  String ret_Token = "";

  HTTPClient http;
  http.begin("https://www.googleapis.com/oauth2/v4/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String param = "";
  param += "refresh_token=" + _refreshToken + "&";
  param += "client_id=" + _clientID + "&";
  param += "client_secret=" + _clientSecret + "&";
  param += "grant_type=refresh_token";

  int code = http.POST(param);
  Serial.println(code);
  if (code > 0) {
    String response = http.getString();
    Serial.println(response);
  }

  return ret_Token;
}

void setup() {
  Nefry.setProgramName("Get GoogleAPI AccessToken");

  Nefry.enableSW();

  Nefry.setStoreTitleStr("Refresh Token", NEFRY_DATASTORE_REFRESH_TOKEN);
  Nefry.setStoreTitleStr("Client ID", NEFRY_DATASTORE_CLIENT_ID);
  Nefry.setStoreTitleStr("Client Secret", NEFRY_DATASTORE_CLIENT_SECRET);

}

void loop() {
  if (Nefry.readSW()) {
    getAccessToken(Nefry.getStoreStr(NEFRY_DATASTORE_REFRESH_TOKEN),
                   Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_ID),
                   Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_SECRET));
  }

}
