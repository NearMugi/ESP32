//JSONデータをサイトから取得、その情報を基にアウトプットする
#include <Nefry.h>
#include <HTTPClient.h> // HTTP-GETのために必要

#define URL "http://rti-giken.jp/fhc/api/train_tetsudo/delay.json"  //遅延情報JSONデータ
#define TARGET "山手線"  //遅延しているか(文字列に含まれているか)チェックする路線

void setup() {

}

void loop() {
  String trainDelayJson = getTrainDelayJson();
  
  if (trainDelayJson.indexOf(TARGET) >= 0) {
    blinkFor30Sec(255, 0, 0); // 赤く光らせる
  } else {
    blinkFor30Sec(0, 0, 255); // 青く光らせる
  }
}

String getTrainDelayJson() {
  String payload = ""; // 運行情報を保存しておく
  HTTPClient http; // HTTP-GETをするために必要な宣言
  int httpCode; // HTTP-GETの結果（int）を保存しておく

  // 運行情報のページにアクセス　※https:// だと簡単に取得できないらしい
  http.begin(URL);

  // その結果を保存
  httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    // 成功だったら、運行情報の文字列を取得して保存
    payload = http.getString();
    Nefry.println("[接続成功]");
    Nefry.println(payload);
  } else {
    // 失敗だったら、エラーコードをログに出力
    Nefry.println("[接続失敗]");
    Nefry.println(http.errorToString(httpCode));
  }

  return payload;
}

// 30秒間、500msecおきに点灯と消灯を繰り返します
void blinkFor30Sec(int r, int g, int b) {
  for (int i = 0; i < 30; i++) {
    Nefry.setLed(r, g, b);
    Nefry.ndelay(500);
    Nefry.setLed(0, 0, 0);
    Nefry.ndelay(500);
  }
}

