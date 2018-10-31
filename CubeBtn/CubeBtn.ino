//磁気センサーを用いた、キューブ型ボタン

#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);


//++++++++++++++++++++++++++++++++++++++++++++
//プロジェクト全体の定義
//++++++++++++++++++++++++++++++++++++++++++++
//ループ周期(us)
#include "interval.h"
#define LOOPTIME_DISP  100000
#define LOOPTIME_JIKI  10000
#define LOOPTIME_BTN   100000
#define LOOPTIME_LED   100000
#define LOOPTIME_MQTT  10000


//++++++++++++++++++++++++++++++++++++++++++++
//ディスプレイ
//++++++++++++++++++++++++++++++++++++++++++++
String ipStr;
String MsgMqtt;
String MsgPublishData;

//++++++++++++++++++++++++++++++++++++++++++++
//折れ線グラフ
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphLine.h"
//static変数を定義
int graph_line::valueSIZE;

//折れ線グラフの領域
#define GRAPH_LINE_POS_X 27
#define GRAPH_LINE_POS_Y 10
#define GRAPH_LINE_LEN_X 100
#define GRAPH_LINE_LEN_Y 50
#define GRAPH_LEN_DPP 10 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

#define VALUE_LINE_MIN 0
#define VALUE_LINE_MAX 1023
#define LINE_PLOT_SIZE (GRAPH_LINE_LEN_X / GRAPH_LEN_DPP) + 1
int x[LINE_PLOT_SIZE];  //x座標
int t[LINE_PLOT_SIZE];  //一定間隔に垂線を引くための配列(垂線の有無)
graph_line grline = graph_line(
                      LOOPTIME_DISP,
                      GRAPH_LINE_POS_X,
                      GRAPH_LINE_POS_Y,
                      GRAPH_LINE_LEN_X,
                      GRAPH_LINE_LEN_Y,
                      GRAPH_LEN_DPP,
                      VALUE_LINE_MIN,
                      VALUE_LINE_MAX,
                      LINE_PLOT_SIZE,
                      &x[0],
                      &t[0]
                    );

//グラフの設定
//頂点の数が可変なので外部で配列を用意している
int v1[LINE_PLOT_SIZE]; //頂点の値

//グラフの初期化
void dispGraphLine_init() {
  grline.initGraphTime();
  grline.setGraph(0, &v1[0], VERTEX_CIR);
}

//グラフの描画
void dispGraphLine_update() {
  grline.dispArea();
  grline.updateGraph();
}

//++++++++++++++++++++++++++++++++++++++++++++
//MQTT
//++++++++++++++++++++++++++++++++++++++++++++
#define NEFRY_DATASTORE_BEEBOTTE_CUBEBTN 1
#define BBT "mqtt.beebotte.com"
#define QoS 1
String bbt_token;
#define Channel "CubeBtn"
#define Res "ptn"
char topic[64];
WiFiClient espClient;
PubSubClient client(espClient);

//connect mqtt broker
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE_CUBEBTN);
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

void publish()
{
  //日付を取得する
  time_t  t = time(NULL);

  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();
  
  root["ptn"] = 0;
  root["ispublic"] = true;
  root["ts"] = t;

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  MsgPublishData = String(buffer);

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer, QoS);
}

//date
#include <time.h>
#define JST     3600*9






void setup() {
  Nefry.setProgramName("Trigger CubeButton");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);


  //mqtt
  client.setServer(BBT, 1883);
  sprintf(topic, "%s/%s", Channel, Res);
  Nefry.setStoreTitle("BeeBotte_Token_CubeButton", NEFRY_DATASTORE_BEEBOTTE_CUBEBTN);

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  //IPアドレス(ディスプレイに表示)
  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}

void loop() {
  if (!client.connected()) reconnect();

  //Display
  interval<LOOPTIME_DISP>::run([] {
  });
  //JikiSensor
  interval<LOOPTIME_JIKI>::run([] {
  });
  //Button
  interval<LOOPTIME_BTN>::run([] {
  });
  //LED
  interval<LOOPTIME_LED>::run([] {
  });  
  //MQTT publish
  interval<LOOPTIME_MQTT>::run([] {
  });
}
