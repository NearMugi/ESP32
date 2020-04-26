// GROVE振動センサー(SW-420)の値を読み取ってMQTTに投げる
// https://www.seeedstudio.com/Grove-Vibration-Sensor-SW-420.html

#include "M5Atom.h"
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "interval.h"

// I2c
#define SDA 26
#define SCL 32
bool isOn;

// Wifi
char *ssid = "Buffalo-G-36F0";
char *password = "[password]";

// MQTT
char *BBT = "mqtt.beebotte.com";
int QoS = 0;
char *topic = "CatWatching/vibration";
String bbt_token = "token:[token]";
WiFiClient espClient;
PubSubClient client(espClient);

// date
#include <time.h>
#define JST 3600 * 9

//ループ周期(us)
#define LOOPTIME_MQTT 5000000

bool reconnect()
{
  Serial.print(F("\nAttempting MQTT connection..."));
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  // Attempt to connect
  if (client.connect(clientId.c_str(), bbt_token.c_str(), ""))
  {
    Serial.println("connected");
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    return false;
  }
}

void publish()
{
  // https://arduinojson.org/v6/doc/upgrade/
  char buffer[128];
  StaticJsonDocument<128> root;

  // 日付を取得する
  time_t t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  uint8_t mon = tm->tm_mon + 1;
  uint8_t day = tm->tm_mday;
  uint8_t wd = tm->tm_wday;
  uint8_t hour = tm->tm_hour;

  root["vibration"] = true;
  root["ispublic"] = false;
  root["ts"] = t;
  serializeJson(root, buffer);
  client.publish(topic, buffer, QoS);

  Serial.println(String(buffer));
}

void chkVibrationSensor()
{
  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0)
    {
      isOn = true;
      break;
    }
  }
}

void setup()
{
  // Serial / i2c / LED
  M5.begin(false, true, true);
  Wire.begin(SDA, SCL);

  //Wifi
  Serial.print(F("Wifi "));
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(F(" Connect"));

  // MQTT
  client.setServer(BBT, 1883);

  // date
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  isOn = false;
}

void loop()
{
  // MQTT Clientへ接続
  if (!client.connected())
  {
    M5.dis.drawpix(0, 0x00f000);
    if (!reconnect())
      return;
  }

  // 振動センサーから値を読み取る
  // 一度ONになったらpublishされるまで読み取らない
  if (isOn)
  {
    M5.dis.drawpix(0, 0x0000f0);
  }
  else
  {
    M5.dis.drawpix(0, 0xf00000);
    chkVibrationSensor();
  }

  //MQTT publish
  interval<LOOPTIME_MQTT>::run([] {
    if (isOn)
    {
      publish();
      isOn = false;
    }
  });

  M5.update();
}
