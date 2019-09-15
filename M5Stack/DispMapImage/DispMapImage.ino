//#define DEBUG

#include <WiFi.h>
#include <M5Stack.h>
#include <PubSubClient.h>
#include "interval.h"
#include <ArduinoJson.h>

//Wifi
char *ssid = "Buffalo-G-36F0";
char *password = "[パスワード]";

//MQTT
char *BBT = "mqtt.beebotte.com";
int QoS = 1;
char *topic = "GPS/axis";
String bbt_token;
String ts;
String getAxis;
String oldGetAxis;

//http
const String host = "[プロジェクト名].cloudfunctions.net";
const String function = "GPSMap_getMapBinary";
const int httpsPort = 443;
const String apikey = "";

//Image
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;
#include <ArduinoJson.h>

bool isGet;

//button
const uint8_t buttonA_GPIO = 39;
const uint8_t buttonB_GPIO = 38;
const uint8_t buttonC_GPIO = 37;

WiFiClient espClient;
PubSubClient client(espClient);

//date
#include <time.h>
#define JST 3600 * 9

void reconnect()
{
  Serial.print(F("\nAttempting MQTT connection..."));
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  // Attempt to connect
  if (client.connect(clientId.c_str(), bbt_token.c_str(), ""))
  {
    Serial.println(F("connected"));
    client.subscribe(topic);
  }
  else
  {
    Serial.print(F("failed, rc="));
    delay(5000);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  if (length <= 0)
  {
    isGet = false;
    return;
  }

  //データを取得
  char getPayload[length];
  for (int i = 0; i < length; i++)
  {
    getPayload[i] = '\0';
  }
  for (int i = 0; i < length; i++)
  {
    getPayload[i] = (char)payload[i];
  }

  StaticJsonDocument<700> doc;
  DeserializationError error = deserializeJson(doc, getPayload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    isGet = false;
    return;
  }
  const char *tmpTs = doc["ts"];
  ts = tmpTs;

  getAxis = "";
  const char *tmpAxis = doc["axis"];
  getAxis = tmpAxis;

  if (getAxis != oldGetAxis)
  {
    isGet = true;
  }
  else
  {
    isGet = false;
  }
  oldGetAxis = getAxis;

#ifdef DEBUG
  Serial.println(getAxis);
#endif
}

void dispImage()
{
  //httpリクエストで画像(バイナリデータ)を取得

  //RuntimeConfigのデータを取得する
#ifdef DEBUG
  Serial.println(F("\n++++++++++++++++++++++\n[RuntimeConfig]\n++++++++++++++++++++++\n"));
#endif
  String ret = cfs.getRuntimeConfig(host, "googleAPI");

  //取得したjsonデータから欲しい情報を取得する
  String GPSApiKey = cfs.getJsonValue(ret, "GPSApiKey");

  //GPSMap_getMapBinaryを使う
#ifdef DEBUG
  Serial.println(F("\n++++++++++++++++++++++\n[GPSMap_getMapBinary]\n++++++++++++++++++++++\n"));
#endif
  String _axis = getAxis;
  String postData = "";
  postData += "{\"key\" : \"" + GPSApiKey + "\",";
  postData += "\"ts\" : \"" + ts + "\",";
  postData += "\"axis\" : \"" + _axis + "\",";
  postData += "\"color\" : \"true\",";
  postData += "\"trim\" : \"@\"";
  postData += "}";

  Serial.println(F("draw ImageData"));
  drawImage(postData, "1", 0, 0);
  drawImage(postData, "2", 80, 0);
  drawImage(postData, "3", 160, 0);
  drawImage(postData, "4", 240, 0);

  drawImage(postData, "5", 0, 120);
  drawImage(postData, "6", 80, 120);
  drawImage(postData, "7", 160, 120);
  drawImage(postData, "8", 240, 120);
}

void drawImage(String _postData, String _trimNo, int _posX, int _posY)
{
  _postData.replace("@", _trimNo);
  String retImage = cfs.callCloudFunctions_String(host, function, _postData);
  int len = retImage.length();
  Serial.print(F("[Get Data] Size : "));
  Serial.println(len);
  //Serial.println(retImage);
  uint8_t bytes[len];
  for (int i = 0; i < len; i++)
  {
    bytes[i] = retImage[i];
    //Serial.print(String(bytes[i],HEX));
    //Serial.print(" ");
  }
  M5.Lcd.drawJpg(bytes, len, _posX, _posY);
}

void debugDisp(String _axis)
{
  //日付を取得する
  time_t t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  char _fn[15] = "";
  sprintf(_fn, "%02d/%02d %02d:%02d:%02d", tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  
  ts = String(_fn);
  getAxis = _axis;
  isGet = true;
  //Serial.println(ts);
}

void setup()
{
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setBrightness(120); // BRIGHTNESS = MAX 255
  M5.Lcd.fillScreen(BLACK);  // CLEAR SCREEN
  M5.Lcd.setRotation(1);     // SCREEN ROTATION = 0

  M5.Lcd.setCursor(50, 120);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("Waiting...");

  //Wifi
  Serial.print(F("Wifi "));
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(F(" Connect"));

  //http
  cfs.InitAPI();

  //MQTT
  String ret = cfs.getRuntimeConfig(host, "MQTT");
  //取得したjsonデータから欲しい情報を取得する
  bbt_token = "";
  bbt_token += "token:";
  bbt_token += cfs.getJsonValue(ret, "GPS");
  client.setServer(BBT, 1883);
  client.setCallback(callback);

  //Image
  isGet = false;
  oldGetAxis = "";

  //button
  pinMode(buttonA_GPIO, INPUT);
  pinMode(buttonB_GPIO, INPUT);
  pinMode(buttonC_GPIO, INPUT);

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void loop()
{
  //接続,Subscribe
  if (!client.connected())
    reconnect();
  client.loop();

  //画像の表示
  interval<1000000>::run([] {
    if (isGet)
    {
      M5.Lcd.fillScreen(BLACK); // CLEAR SCREEN
      M5.Lcd.setCursor(50, 120);
      M5.Lcd.setTextSize(3);
      M5.Lcd.setTextColor(ORANGE);
      M5.Lcd.print("MAP Updating...");
      dispImage();
    }
    isGet = false;
  });

  //デバッグ用
  if (M5.BtnA.wasPressed())
  {
    debugDisp("35.6585733,139.7452599");
  }

  if (M5.BtnB.wasPressed())
  {
    debugDisp("35.7097728,139.8095165");
  }

  if (M5.BtnC.wasPressed())
  {
    debugDisp("35.6822647,139.7714705");
  }

  M5.update();
}
