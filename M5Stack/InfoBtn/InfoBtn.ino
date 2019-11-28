//#define DEBUG

#include <WiFi.h>
#include <M5Stack.h>
#include <PubSubClient.h>
#include "interval.h"
#include <ArduinoJson.h>

//Wifi
char *ssid = "Buffalo-G-36F0";
//char *password = "[パスワード]";

//http
//const String host = "[プロジェクト名].cloudfunctions.net";
const String function = "getDriveImage_M5stack";
const int httpsPort = 443;

//Image
#include "googleCloudFunctions.h"
googleCloudFunctions cfs;
#include <ArduinoJson.h>

//button
const uint8_t buttonA_GPIO = 39;
const uint8_t buttonB_GPIO = 38;
const uint8_t buttonC_GPIO = 37;

WiFiClient espClient;
PubSubClient client(espClient);

void dispImage(String imgFile)
{
  //httpリクエストで画像(バイナリデータ)を取得
  String postData = "{ \"drive\" : ";
  postData += "{\"img\" : \"";
  postData += imgFile;
  postData += "\",";
  postData += "\"trim\" : \"@\"";
  postData += "}}";

  Serial.println(F("draw ImageData"));
  drawImage(postData, "1", 0, 0);
  drawImage(postData, "5", 0, 120);

  drawImage(postData, "2", 80, 0);
  drawImage(postData, "6", 80, 120);

  drawImage(postData, "3", 160, 0);
  drawImage(postData, "7", 160, 120);

  drawImage(postData, "4", 240, 0);
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

  //button
  pinMode(buttonA_GPIO, INPUT);
  pinMode(buttonB_GPIO, INPUT);
  pinMode(buttonC_GPIO, INPUT);

  // 初期画面
  draw("imgWeather.jpeg", "Weather");
}

void draw(String imgFile, String msg)
{
  M5.Lcd.fillScreen(BLACK); // CLEAR SCREEN
  M5.Lcd.setCursor(50, 120);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print(msg + " Updating...");
  dispImage(imgFile);
}

void loop()
{
  if (M5.BtnA.wasPressed())
  {
    draw("imgCalendar.jpeg", "Calendar");
  }

  if (M5.BtnB.wasPressed())
  {
    draw("imgWeather.jpeg", "Weather");
  }

  if (M5.BtnC.wasPressed())
  {
    draw("imgCalendar_Demo.jpeg", "Demo");
  }

  M5.update();
}
