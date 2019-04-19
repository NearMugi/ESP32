#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <time.h>
#define JST     3600*9

#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

String mqtt_server;

#define BBT "mqtt.beebotte.com"
#define QoS 0
String bbt_token;
#define Channel "FoodMenu"
#define Res "date"
char topic[64];

WiFiClient espClient;
PubSubClient client(espClient);
String msg;

#define MAX_MSGSIZE 20
String getTopic;
char getPayload[MAX_MSGSIZE];

String MsgMqtt;

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(1);
  const char* tmp = bbt_token.c_str();
  // Attempt to connect
  if (client.connect(clientId.c_str(), tmp, "")) {
    Serial.println("connected");
    MsgMqtt = "Mqtt Connected";
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    MsgMqtt = "Mqtt DisConnected";
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  getTopic = (String)topic;
  for (int i = 0; i < MAX_MSGSIZE; i++) {
    getPayload[i] = '\0';
  }
  for (int i = 0; i < length; i++) {
    getPayload[i] = (char)payload[i];
  }
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  String text;
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, MsgMqtt);
  NefryDisplay.drawString(0, 15, msg);

  NefryDisplay.display();
  Nefry.ndelay(10);
}

void setup() {
  Nefry.enableSW();
  Nefry.setProgramName("BeeBotte Publish Submit Test");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);

  client.setServer(BBT, 1883);
  client.setCallback(callback);

  sprintf(topic, "%s/%s", Channel, Res);

  Nefry.setStoreTitle("MQTTServerIP", 0); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", 1);
  //  mqtt_server = Nefry.getStoreStr(0);

  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

}

void loop() {

  if (!client.connected()) reconnect();

  if (Nefry.readSW()) {
    msg = "Switch On";
    if (client.connected()) {
      client.loop();
      publish(Res, msg);
    }
  }

}

void publish(const char* resource, String data)
{
  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();
  root["data"] = data;
  root["ispublic"] = true;
  root["ts"] = time(NULL) ;

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer, QoS);
}

