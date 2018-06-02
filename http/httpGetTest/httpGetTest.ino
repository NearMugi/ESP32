#include <Nefry.h>
#include <HTTPClient.h>

const char URL[] = "http://192.168.0.9:8000";  //"http://unkou.keikyu.co.jp/" なら取得できる

void setup() {
  String result1 = getPageSource((char*)URL);
  Serial.println(result1);
}

void loop() {

}

String getPageSource(char host[]) {
  Serial.println(host);
  HTTPClient http;

  http.begin(host);
//  http.begin("192.168.0.9", 8000, "/"); //うまくいかない。
  int httpCode = http.GET();

  String result = "";

  if (httpCode < 0) {
    result = http.errorToString(httpCode);
  } else if (http.getSize() < 0) {
    result =  "size is invalid";
  } else {
    result = http.getString();
  }

  http.end();
  return result;
}
