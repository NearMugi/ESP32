#include <Nefry.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "mqttConfig.h"

// ループ周期(ms)

// MQTT
#define NEFRY_DATASTORE_BEEBOTTE 1
const char *host = "mqtt.beebotte.com";
int QoS = 0;
const char *clientId;
String bbt_token;
WiFiClientSecure espClient;
PubSubClient mqttClient(host, 8883, espClient);
bool isConnect;

bool reconnect()
{
  Serial.print(F("\nAttempting MQTT connection..."));
  // Attempt to connect
  const char *user = bbt_token.c_str();
  if (mqttClient.connect(clientId, user, NULL))
  {
    Serial.println("connected");
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
  }
  return mqttClient.connected();
}

void setup()
{
  Nefry.setStoreTitle("MQTT_Token", NEFRY_DATASTORE_BEEBOTTE);

  // MQTT
  espClient.setCACert(beebottle_ca_cert);
  uint64_t chipid = ESP.getEfuseMac();
  String tmp = "ESP32-" + String((uint16_t)(chipid >> 32), HEX);
  clientId = tmp.c_str();
  Serial.println(tmp);

  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE);
  Serial.println(bbt_token);
}

void loop()
{
  // MQTT Clientへ接続
  if (!mqttClient.connected())
    isConnect = reconnect();
  if (isConnect)
    mqttClient.loop();
}
