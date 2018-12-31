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

void setup() {
  Nefry.setProgramName("Get GoogleAPI AccessToken");

  Nefry.setStoreTitleStr("Refresh Token", NEFRY_DATASTORE_REFRESH_TOKEN);
  Nefry.setStoreTitleStr("Client ID", NEFRY_DATASTORE_CLIENT_ID);
  Nefry.setStoreTitleStr("Client Secret", NEFRY_DATASTORE_CLIENT_SECRET);

  refresh_token = Nefry.getStoreStr(NEFRY_DATASTORE_REFRESH_TOKEN);
  client_id = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_ID);
  client_secret = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_SECRET);

  delay(5000);

  accessToken = api.GetAccessToken(refresh_token, client_id, client_secret);
  Serial.println(accessToken);

}

void loop() {
}
