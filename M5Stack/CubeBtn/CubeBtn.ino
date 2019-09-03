//磁気センサーを用いた、キューブ型ボタン
//publishするときにtopicを変える
//1～4:早起きボタン
//5:給食
//6:？？？

#include <WiFi.h>
#include <M5Stack.h>
#include <PubSubClient.h>
#include "interval.h"
#include "googleCloudFunctions.h"
#include <ArduinoJson.h>

//++++++++++++++++++++++++++++++++++++++++++++
//プロジェクト全体の定義
//++++++++++++++++++++++++++++++++++++++++++++
//使用ピン
#define PIN_JIKI 14
#define PIN_BTN 16

//ループ周期(us)
#define LOOPTIME_DISP 100000
#define LOOPTIME_JIKI 30000
#define LOOPTIME_BTN 50000
#define LOOPTIME_MQTT 500000

//ステータス
#define STATUS_NONE "NONE"
#define STATUS_TAIKI "WAIT"
#define STATUS_BTN_ON "PRESS"
#define STATUS_MQTT_SUC "SEND"
String nowStatus = STATUS_TAIKI;

//MQTT送信中の待ち時間(ms)
#define WAIT_MQTT_PUBLISH 5000
unsigned long waitingTime;

//Wifi
char *ssid = "Buffalo-G-36F0";
char *password = "[password]";

//googleCloudFunctions
const String host = "[Project Name].cloudfunctions.net";
googleCloudFunctions cfs;

//date
#include <time.h>
#define JST 3600 * 9

//button
const uint8_t buttonA_GPIO = 39;
const uint8_t buttonB_GPIO = 38;
const uint8_t buttonC_GPIO = 37;


//++++++++++++++++++++++++++++++++++++++++++++
//磁気センサ(キューブ)
//++++++++++++++++++++++++++++++++++++++++++++
#define JIKI_DEF 0
#define JIKI_PTN1 1
#define JIKI_PTN2 2
#define JIKI_PTN3 3
#define JIKI_PTN4 4
#define JIKI_PTN5 5
#define JIKI_PTN6 6
#define JIKI_NONE 9

//閾値
int jikiPtnDef[2] = {2650, 2750};
int jikiPtn1[2] = {1550, 1900};
int jikiPtn2[2] = {2100, 2300};
int jikiPtn3[2] = {2500, 2650};
int jikiPtn4[2] = {2750, 2900};
int jikiPtn5[2] = {2950, 3650};
int jikiPtn6[2] = {3700, 4098};

//保存するデータ数は1秒分
#define JIKI_SIZE (1000000 / LOOPTIME_JIKI)
int jiki[JIKI_SIZE];
int jikiAvg;
int jikiPtn = JIKI_NONE;

//++++++++++++++++++++++++++++++++++++++++++++
//磁気センサ(ボタン)
//++++++++++++++++++++++++++++++++++++++++++++
#define jikiBtn 2100 //ONと判断する閾値
#define BTN_CNT 5    //チャタリング対策
int btnCnt = 0;

//++++++++++++++++++++++++++++++++++++++++++++
//MQTT
//++++++++++++++++++++++++++++++++++++++++++++
char *BBT = "mqtt.beebotte.com";
int QoS = 1;
char *topicUser = "CubeButton/user";
char *topicFood = "CubeButton/food";
String bbt_token = "";
WiFiClient espClient;
PubSubClient client(espClient);

void reconnect()
{
  Serial.print(F("\nAttempting MQTT connection..."));
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.print(F("MQTT connection..."));    
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  // Attempt to connect
  if (client.connect(clientId.c_str(), bbt_token.c_str(), ""))
  {
    Serial.println("connected");
    M5.Lcd.setCursor(0, 125);
    M5.Lcd.print(F("connected"));   
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    M5.Lcd.setCursor(0, 125);
    M5.Lcd.print(F("failed..."));   
  }
}

void publish()
{
  //https://arduinojson.org/v6/doc/upgrade/
  char buffer[128];
  StaticJsonDocument<128> root;

  //日付を取得する
  time_t t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  uint8_t mon = tm->tm_mon + 1;
  uint8_t day = tm->tm_mday;
  uint8_t wd = tm->tm_wday;
  uint8_t hour = tm->tm_hour;

  //パターンごとに送信するトピックを変更する
  switch (jikiPtn)
  {
  //早起きボタン
  case JIKI_PTN1:
  case JIKI_PTN2:
  case JIKI_PTN3:
  case JIKI_PTN4:
    root["user"] = jikiPtn;
    root["ispublic"] = true;
    root["ts"] = t;

    // Now print the JSON into a char buffer
    serializeJson(root, buffer);

    // Now publish the char buffer to Beebotte
    client.publish(topicUser, buffer, QoS);
    M5.Lcd.fillScreen(BLACK);  // CLEAR SCREEN
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print(topicUser);    
    M5.Lcd.setCursor(0, 25);
    M5.Lcd.print(buffer);    
    break;

  //給食
  case JIKI_PTN5:
  {
    uint8_t plusDay = 0;
    if (wd == 0)
      plusDay = 1; //日曜日だった場合は月曜日の日付にする。
    if (hour >= 20)
      plusDay = 1; //20時以降だった場合は翌日の日付にする。
    if (wd == 6 && hour >= 20)
      plusDay = 2; //土曜日、かつ20時以降だった場合は月曜日の日付にする。
    if (plusDay > 0)
    {
      t += 86400 * plusDay;
      tm = localtime(&t);
      mon = tm->tm_mon + 1;
      day = tm->tm_mday;
    }

    char date[4];
    sprintf(date, "%02d%02d", mon, day);

    root["food"] = date;
    root["ispublic"] = true;
    root["ts"] = t;

    // Now print the JSON into a char buffer
    serializeJson(root, buffer);

    // Now publish the char buffer to Beebotte
    client.publish(topicFood, buffer, QoS);
    M5.Lcd.fillScreen(BLACK);  // CLEAR SCREEN
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print(topicFood);    
    M5.Lcd.setCursor(0, 25);
    M5.Lcd.print(buffer);    
    break;
  }
  case JIKI_PTN6:
    break;
  }

  Serial.println(String(buffer));
}

void setup()
{

  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setBrightness(120); // BRIGHTNESS = MAX 255
  M5.Lcd.fillScreen(BLACK);  // CLEAR SCREEN
  M5.Lcd.setRotation(1);     // SCREEN ROTATION = 0
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(ORANGE);

  //Wifi
  Serial.print(F("Wifi "));
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(F(" Connect"));
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(F("Wifi Connect"));

  //googleCloudFunctions
  cfs.InitAPI();

  //pin
  pinMode(PIN_JIKI, INPUT);
  pinMode(PIN_BTN, INPUT);
  //button
  pinMode(buttonA_GPIO, INPUT);
  pinMode(buttonB_GPIO, INPUT);
  pinMode(buttonC_GPIO, INPUT);

  //mqtt
  String ret = cfs.getRuntimeConfig(host, "MQTT");
  //取得したjsonデータから欲しい情報を取得する
  bbt_token = "";
  bbt_token += "token:";
  bbt_token += cfs.getJsonValue(ret, "CubeButton");
  client.setServer(BBT, 1883);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.print(bbt_token);

  //date
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void loopDisplay()
{
  if (nowStatus == STATUS_NONE)
  {
    //何もなし
  }

  //キューブを置いている段階 or ボタンを押したとき
  if (nowStatus == STATUS_TAIKI || nowStatus == STATUS_BTN_ON)
  {
  }

  //MQTTにパブリッシュ中
  if (nowStatus == STATUS_MQTT_SUC)
  {
  }
}

void loopJikiSensor()
{
  //データ保存と平均値の算出
  int _v = analogRead(PIN_JIKI);
  //Serial.println(_v);
  jikiAvg = 0;
  for (int i = 0; i < JIKI_SIZE - 1; i++)
  {
    jiki[i] = jiki[i + 1];
    jikiAvg += jiki[i + 1];
  }

  jiki[JIKI_SIZE - 1] = _v;
  jikiAvg += jiki[JIKI_SIZE - 1];
  jikiAvg /= JIKI_SIZE;

  //パターンの判定
  if (jikiAvg >= jikiPtnDef[0] && jikiAvg <= jikiPtnDef[1])
  {
    jikiPtn = JIKI_DEF;
    return;
  }
  if (jikiAvg >= jikiPtn1[0] && jikiAvg <= jikiPtn1[1])
  {
    jikiPtn = JIKI_PTN1;
    return;
  }
  if (jikiAvg >= jikiPtn2[0] && jikiAvg <= jikiPtn2[1])
  {
    jikiPtn = JIKI_PTN2;
    return;
  }
  if (jikiAvg >= jikiPtn3[0] && jikiAvg <= jikiPtn3[1])
  {
    jikiPtn = JIKI_PTN3;
    return;
  }
  if (jikiAvg >= jikiPtn4[0] && jikiAvg <= jikiPtn4[1])
  {
    jikiPtn = JIKI_PTN4;
    return;
  }
  if (jikiAvg >= jikiPtn5[0] && jikiAvg <= jikiPtn5[1])
  {
    jikiPtn = JIKI_PTN5;
    return;
  }
  if (jikiAvg >= jikiPtn6[0] && jikiAvg <= jikiPtn6[1])
  {
    jikiPtn = JIKI_PTN6;
    return;
  }

  //どの条件にも引っかからなかった
  jikiPtn = JIKI_NONE;
  return;
}

void loopBtn()
{
  //待機以外の時はボタンを無効にする
  if (nowStatus != STATUS_TAIKI)
  {
    btnCnt = 0;
    return;
  }

  int btn = digitalRead(PIN_BTN);

  //閾値を超えた複数回認識してからONにする
  if (btn > jikiBtn)
  {
    if (++btnCnt >= BTN_CNT)
    {
      nowStatus = STATUS_BTN_ON;
    }
  }
  else
  {
    btnCnt = 0;
  }
}

void loopMQTT()
{
  //送信中の待ち
  if (nowStatus == STATUS_MQTT_SUC)
  {
    unsigned long _t = millis();
    if (abs(_t - waitingTime) >= WAIT_MQTT_PUBLISH)
    {
      nowStatus = STATUS_TAIKI;
    }
    return;
  }

  //ボタンを押した直後
  if (nowStatus == STATUS_BTN_ON)
  {

    //どの面が上なのか分からない
    if (jikiPtn == JIKI_NONE)
    {
      nowStatus = STATUS_TAIKI;
      return;
    }

    //接続できていない。
    if (!client.connected())
    {
      nowStatus = STATUS_TAIKI;
      return;
    }

    publish();
    nowStatus = STATUS_MQTT_SUC;
    waitingTime = millis();
  }
}

void loop()
{
  if (!client.connected())
    reconnect();

  //MQTT publish
  interval<LOOPTIME_MQTT>::run([] {
    loopMQTT();
  });

#if false
  //JikiSensor
  interval<LOOPTIME_JIKI>::run([] {
    loopJikiSensor();
  });

  //Button
  interval<LOOPTIME_BTN>::run([] {
    loopBtn();
  });

  //Display
  interval<LOOPTIME_DISP>::run([] {
    loopDisplay();
  });
#endif

  //デバッグ用
  if (M5.BtnA.wasPressed())
  {
    jikiPtn = JIKI_PTN1;
    nowStatus = STATUS_BTN_ON;
  }

  if (M5.BtnB.wasPressed())
  {
    jikiPtn = JIKI_PTN5;
    nowStatus = STATUS_BTN_ON;
  }

  if (M5.BtnC.wasPressed())
  {
    jikiPtn = JIKI_PTN6;
    nowStatus = STATUS_BTN_ON;
  }


  M5.update();
}
