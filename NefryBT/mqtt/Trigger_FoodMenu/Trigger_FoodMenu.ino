#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

//mqtt
#define NEFRY_DATASTORE_BEEBOTTE_FOODMENU 0
#define BBT "mqtt.beebotte.com"
#define QoS 1
String bbt_token;
#define Channel "FoodMenu"
#define Res "date"
char topic[64];
WiFiClient espClient;
PubSubClient client(espClient);

//date
#include <time.h>
#define JST     3600*9

//switch
#define PIN_DIGITAL_SW D8
#define MAX_CNT 5
#define WAIT_NEXTTIME_MS 5000 //publishからの待ち時間(ms)
uint8_t cntOn;
bool sw;
long waitTime; //一度publishしてからの待ち時間

//NefryDisplayMessage
String MsgMqtt;
String MsgSw;
String MsgPublishData;
String ipStr;

//ループ周期(us)
#include <interval.h>
#define LOOP_TIME_US 10000

//connect mqtt broker
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

  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  //mqtt
  client.setServer(BBT, 1883);
  sprintf(topic, "%s/%s", Channel, Res);
  Nefry.setStoreTitle("BeeBotte_Token_FoodMenu", NEFRY_DATASTORE_BEEBOTTE_FOODMENU);

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  //switch
  pinMode(PIN_DIGITAL_SW, INPUT);
  cntOn = 0;
  sw = false;
  waitTime = 0;
  MsgSw = "";

  //displayMessage
  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}

void loop() {
  if (!client.connected()) reconnect();

  //switch and publish
  interval<LOOP_TIME_US>::run([] {
    //スイッチがON
    if (sw) {
      MsgSw = "Wait Next Time...";
      //次のPublish送信が有効になるまで待ち
      if ((millis() - waitTime) > WAIT_NEXTTIME_MS) {
        sw = false;
        cntOn = 0;
      }
    } else {
      MsgSw = "Switch Off";
      if (digitalRead(PIN_DIGITAL_SW)) {
        if (++cntOn >= MAX_CNT) {
          //スイッチがONになったタイミングでPublish
          sw = true;
          waitTime = millis();

          publish();
        }
      } else {
        sw = false;
        cntOn = 0;
      }
    }

  });
}

void publish()
{
  //日付を取得する
  time_t  t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  uint8_t mon = tm->tm_mon + 1;
  uint8_t day = tm->tm_mday;
  uint8_t wd = tm->tm_wday;
  uint8_t hour = tm->tm_hour;

  uint8_t plusDay = 0;
  if (wd == 0) plusDay = 1;   //日曜日だった場合は月曜日の日付にする。
  if (hour >= 20) plusDay = 1; //20時以降だった場合は翌日の日付にする。
  if (wd == 6 && hour >= 20) plusDay = 2; //土曜日、かつ20時以降だった場合は月曜日の日付にする。
  if (plusDay > 0) {
    t += 86400 * plusDay;
    tm = localtime(&t);
    mon = tm->tm_mon + 1;
    day = tm->tm_mday;
  }

  char date[4];
  sprintf(date, "%02d%02d", mon, day);

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

