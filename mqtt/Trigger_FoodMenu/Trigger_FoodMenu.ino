#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

String ipStr;

#define NEFRY_DATASTORE_MOSQUITTO 0
#define NEFRY_DATASTORE_BEEBOTTE 1
#define NEFRY_DATASTORE_BEEBOTTE_FOODMENU 2

long nowTime;

#define PIN_DIGITAL_SW D8
#define LOOP_TIME_US 10000  //ループ時間(us)
#define MAX_CNT 5
#define WAIT_NEXTTIME_US 5000000 //publishからの待ち時間(us)
long lpTime;
int cntOn;
bool sw;
long waitTime; //一度publishしてからの待ち時間




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

//NefryDisplayMessage
String MsgMqtt;
String MsgSw;
String MsgPublishData;

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE_FOODMENU);
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

void DispNefryDisplay() {
  NefryDisplay.clear();
  String text;
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, ipStr);
  NefryDisplay.drawString(0, 10, MsgMqtt);
  NefryDisplay.drawString(0, 20, MsgSw);
  NefryDisplay.drawString(0, 30, MsgPublishData);

  NefryDisplay.display();
  Nefry.ndelay(10);
}

void setup() {
  Nefry.setProgramName("Trigger FoodMenu");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);

  client.setServer(BBT, 1883);

  sprintf(topic, "%s/%s", Channel, Res);

  Nefry.setStoreTitle("MQTTServerIP", NEFRY_DATASTORE_MOSQUITTO); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", NEFRY_DATASTORE_BEEBOTTE);
  Nefry.setStoreTitle("BeeBotte_Token_FoodMenu", NEFRY_DATASTORE_BEEBOTTE_FOODMENU);

  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  //switch
  pinMode(PIN_DIGITAL_SW, INPUT);
  lpTime = 0;
  cntOn = 0;
  sw = false;
  waitTime = 0;
  MsgSw = "";

  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}

void loop() {
  nowTime = micros();
  if (!client.connected()) reconnect();

  //switch and publish
  if ((nowTime - lpTime) >  LOOP_TIME_US) {
    lpTime = micros();
    //スイッチがON
    if (sw) {
      MsgSw = "[Switch On ]";
      //次のPublish送信が有効になるまで待ち
      if ((nowTime - waitTime) > WAIT_NEXTTIME_US) {
        sw = false;
        cntOn = 0;
      } else {
        MsgSw = "Wait Next Time";
      }
    } else {
      MsgSw = "[Switch Off]";
      if (digitalRead(PIN_DIGITAL_SW)) {
        if (++cntOn >= MAX_CNT) {
          //スイッチがONになったタイミングでPublish
          sw = true;
          waitTime = micros();

          publish();
        }
      } else {
        sw = false;
        cntOn = 0;
      }
    }
  }

}

void publish()
{
  //日付を取得する
  //※未対応仕様
  //20時以降だった場合は翌日の日付にする。
  //土日だった場合は月曜日の日付にする。
  time_t  t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  int mon = tm->tm_mon + 1;
  int day = tm->tm_mday;
  int wd = tm->tm_wday;
  int hour = tm->tm_hour;

  int plusDay = 0;
  if (wd == 0) plusDay = 1;   //日曜日だった場合は月曜日の日付にする。
  if (hour >= 20) plusDay = 1; //20時以降だった場合は翌日の日付にする。
  if (wd == 6 && hour >= 20) plusDay = 2; //土曜日、かつ20時以降だった場合は月曜日の日付にする。
  if (plusDay > 0) {
    t += 86400 * plusDay;
    tm = localtime(&t);
    mon = tm->tm_mon + 1;
    day = tm->tm_mday;
  }

  char date[5];
  sprintf(date, "%02d,%2d", mon, day);

  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();

  root["data"] = date;
  root["ispublic"] = true;
  root["ts"] = t;

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  MsgPublishData = String(buffer);

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer, QoS);
}

