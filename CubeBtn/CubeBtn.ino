//磁気センサーを用いた、キューブ型ボタン
//publishするときにtopicを変える
//1～4:早起きボタン
//5:給食
//6:？？？

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
//使用ピン
#define PIN_JIKI A3
#define PIN_BTN D8

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_DISP  100000
#define LOOPTIME_JIKI  30000
#define LOOPTIME_BTN   30000
#define LOOPTIME_MQTT  500000
#define LOOPTIME_SLEEP_CNT  1000000


//ステータス
#define STATUS_NONE "NONE"
#define STATUS_TAIKI "WAIT"
#define STATUS_BTN_ON "PRESS"
#define STATUS_MQTT_SUC "SEND"
String nowStatus = STATUS_TAIKI;

//MQTT送信中の待ち時間(ms)
#define WAIT_MQTT_PUBLISH 5000
unsigned long waitingTime;

//++++++++++++++++++++++++++++++++++++++++++++
//スリープ
//++++++++++++++++++++++++++++++++++++++++++++
const unsigned int SLEEP_CNT_S = 3600;  //スリープに入るまでのカウント(秒)
unsigned int sleepCnt;

//++++++++++++++++++++++++++++++++++++++++++++
//磁気センサ
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
//ボタン
//++++++++++++++++++++++++++++++++++++++++++++
#define BTN_OFF 0
#define BTN_DOWN 1
int btnPtn = BTN_OFF;
#define BTN_CNT 5 //チャタリング対策
int btnCnt = 0;

//++++++++++++++++++++++++++++++++++++++++++++
//ディスプレイ
//++++++++++++++++++++++++++++++++++++++++++++
String ipStr;
String MsgMqtt;
String MsgPublishData;
int BtnAniCnt;
int MqttCnt;

//++++++++++++++++++++++++++++++++++++++++++++
//折れ線グラフ
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphBar.h"
//static変数を定義
int graph_bar::valueSIZE;

//棒グラフ(横方向)の領域
#define GRAPH_BARS_POS_X 80
#define GRAPH_BARS_POS_Y 54
#define GRAPH_BARS_LEN_X 40
#define GRAPH_BARS_LEN_Y 10

#define VALUE_BAR_MIN 0
#define VALUE_BAR_MAX 4098 //esp32は分解能12bit
#define BAR_PLOT_SIZE 20  //保存するデータ数
graph_bar grbar;

//グラフの設定
//保存するデータ数が可変なので外部で配列を用意している
//縦方向と横方向は同じデータを使う
int vb1[BAR_PLOT_SIZE];

//グラフの初期化
void dispGraphBarS_init() {
  grbar = graph_bar(
            BAR_SIDE,
            LOOPTIME_DISP,
            GRAPH_BARS_POS_X,
            GRAPH_BARS_POS_Y,
            GRAPH_BARS_LEN_X,
            GRAPH_BARS_LEN_Y,
            VALUE_BAR_MIN,
            VALUE_BAR_MAX,
            BAR_PLOT_SIZE
          );
  grbar.initGraphTime();
  grbar.setGraph(0, &vb1[0]);
}


//棒グラフ(横方向)の描画
void dispGraphBarS_update() {
  grbar.dispArea();
  grbar.updateGraph();
}

//++++++++++++++++++++++++++++++++++++++++++++
//MQTT
//++++++++++++++++++++++++++++++++++++++++++++
#define NEFRY_DATASTORE_BEEBOTTE_CUBEBTN 1
#define BBT "mqtt.beebotte.com"
#define QoS 2
String bbt_token;
#define Channel "CubeButton"
#define ResUser "user"
char topic_user[64];
#define ResFood "food"
char topic_food[64];
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
    MsgMqtt = "Mqtt OK";
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    MsgMqtt = "Mqtt NG";
  }
}

void publish()
{
  char buffer[128];
  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();

  //日付を取得する
  time_t  t = time(NULL);
  struct tm *tm;
  tm = localtime(&t);
  uint8_t mon = tm->tm_mon + 1;
  uint8_t day = tm->tm_mday;
  uint8_t wd = tm->tm_wday;
  uint8_t hour = tm->tm_hour;

  //パターンごとに送信するトピックを変更する
  switch (jikiPtn) {
    //早起きボタン
    case JIKI_PTN1:
    case JIKI_PTN2:
    case JIKI_PTN3:
    case JIKI_PTN4:
      root["user"] = jikiPtn;
      root["ispublic"] = true;
      root["ts"] = t;

      // Now print the JSON into a char buffer
      root.printTo(buffer, sizeof(buffer));

      // Now publish the char buffer to Beebotte
      client.publish(topic_user, buffer, QoS);
      break;

    //給食
    case JIKI_PTN5:
      {
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

        root["food"] = date;
        root["ispublic"] = true;
        root["ts"] = t;

        // Now print the JSON into a char buffer
        root.printTo(buffer, sizeof(buffer));

        // Now publish the char buffer to Beebotte
        client.publish(topic_food, buffer, QoS);
        break;
      }
    case JIKI_PTN6:
      break;
  }

  MsgPublishData = String(buffer);
  Serial.println(MsgPublishData);

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

  //pin
  pinMode(PIN_JIKI, INPUT);
  pinMode(PIN_BTN, INPUT);

  //mqtt
  client.setServer(BBT, 1883);
  sprintf(topic_user, "%s/%s", Channel, ResUser);
  sprintf(topic_food, "%s/%s", Channel, ResFood);
  Nefry.setStoreTitle("Token_CubeButton", NEFRY_DATASTORE_BEEBOTTE_CUBEBTN);

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  //グラフ
  dispGraphBarS_init();
  BtnAniCnt = 0;
  MqttCnt = 0;

  //スリープ
  sleepCnt = 0;
}





void loopDisplay() {

  //IPアドレス
  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

  //グラフデータの更新
  grbar.addGraphData(0, jiki[JIKI_SIZE - 1]);
  grbar.updateGraphTime();

  //描画
  NefryDisplay.clear();

  //ユーザー向けの情報
  if (++BtnAniCnt > 10) BtnAniCnt = 0;
  if (nowStatus == STATUS_NONE) {
    //何もなし
  }

  //キューブを置いている段階 or ボタンを押したとき
  if (nowStatus == STATUS_TAIKI || nowStatus == STATUS_BTN_ON) {
    //次のMQTT送信向けに変数を初期化しておく
    MqttCnt = 0;

    //選択中の番号
    NefryDisplay.drawRect(10, 10, 10, 10);
    NefryDisplay.drawRect(22, 10, 10, 10);
    NefryDisplay.drawRect(34, 10, 10, 10);
    NefryDisplay.drawRect(46, 10, 10, 10);
    NefryDisplay.drawRect(58, 10, 10, 10);
    NefryDisplay.drawRect(70, 10, 10, 10);
    if (jikiPtn >= 1 && jikiPtn <= 6) {
      NefryDisplay.fillRect(10 * jikiPtn + 2 * (jikiPtn - 1), 10, 10, 10);
    }

    //ボタンを押すアニメーション
    NefryDisplay.setFont(ArialMT_Plain_16);
    NefryDisplay.drawString(10, 30, "(`･ω･´)");
    NefryDisplay.fillRect(70, 40, 20, 5);
    if (nowStatus == STATUS_TAIKI) {
      if (jikiPtn >= 1 && jikiPtn <= 6) {
        NefryDisplay.drawString(60, 30, "ﾉｼ");
        NefryDisplay.drawRect(75, 30 + (BtnAniCnt / 2), 10, 10);
      } else {
        NefryDisplay.drawRect(75, 30, 10, 10);
      }
    } else {
      NefryDisplay.drawString(60, 30, " ｼ");
      NefryDisplay.fillRect(70, 40, 20, 5);
      NefryDisplay.drawRect(75, 35, 10, 10);
    }
  }

  //MQTTにパブリッシュ中
  if (nowStatus == STATUS_MQTT_SUC) {
    //Btn
    NefryDisplay.fillRect(10, 40, 20, 5);
    NefryDisplay.drawRect(15, 35, 10, 10);

    //GoogleHomeMini
    NefryDisplay.drawCircle(90, 22, 15);
    NefryDisplay.drawCircle(80, 22, 2);
    NefryDisplay.drawCircle(85, 22, 2);
    NefryDisplay.drawCircle(85, 22, 2);
    NefryDisplay.drawCircle(100, 22, 2);

    //通信
    if (++MqttCnt > 50) MqttCnt = 0;
    NefryDisplay.setFont(ArialMT_Plain_16);
    NefryDisplay.drawString(20 + MqttCnt, 22, ")))");
  }

  //デバッグ用
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 54, String(jikiPtn));
  NefryDisplay.drawString(7, 54, nowStatus);
  NefryDisplay.drawString(0, 44, ipStr);
  NefryDisplay.drawString(60, 44, MsgMqtt);
  NefryDisplay.drawString(60, 54, String(jikiAvg));

  //グラフの描画
  dispGraphBarS_update();

  NefryDisplay.display();
}

void loopJikiSensor() {
  //データ保存と平均値の算出
  int _v = analogRead(PIN_JIKI);
  //Serial.println(_v);
  jikiAvg = 0;
  for (int i = 0; i < JIKI_SIZE - 1; i++) {
    jiki[i] = jiki[i + 1];
    jikiAvg += jiki[i + 1];
  }

  jiki[JIKI_SIZE - 1] = _v;
  jikiAvg += jiki[JIKI_SIZE - 1];
  jikiAvg /= JIKI_SIZE;

  //パターンの判定
  if (jikiAvg >= jikiPtnDef[0] && jikiAvg <= jikiPtnDef[1]) {
    jikiPtn = JIKI_DEF;
    return;
  }
  if (jikiAvg >= jikiPtn1[0] && jikiAvg <= jikiPtn1[1]) {
    jikiPtn = JIKI_PTN1;
    return;
  }
  if (jikiAvg >= jikiPtn2[0] && jikiAvg <= jikiPtn2[1]) {
    jikiPtn = JIKI_PTN2;
    return;
  }
  if (jikiAvg >= jikiPtn3[0] && jikiAvg <= jikiPtn3[1]) {
    jikiPtn = JIKI_PTN3;
    return;
  }
  if (jikiAvg >= jikiPtn4[0] && jikiAvg <= jikiPtn4[1]) {
    jikiPtn = JIKI_PTN4;
    return;
  }
  if (jikiAvg >= jikiPtn5[0] && jikiAvg <= jikiPtn5[1]) {
    jikiPtn = JIKI_PTN5;
    return;
  }
  if (jikiAvg >= jikiPtn6[0] && jikiAvg <= jikiPtn6[1]) {
    jikiPtn = JIKI_PTN6;
    return;
  }

  //どの条件にも引っかからなかった
  jikiPtn = JIKI_NONE;
  return;
}

void loopBtn() {
  bool btn = digitalRead(PIN_BTN);

  //チャタリング対策
  //押したとき複数回認識してからONにする
  if (btn) {
    if (++btnCnt >= BTN_CNT) {
      btn = true;
      btnCnt = BTN_CNT;
    } else {
      btn = false;
    }
  }

  if (btn) {
    Nefry.setLed(128, 0, 255);
  } else {
    Nefry.setLed(0, 255, 0);
  }

  //MQTT送信中の時はボタンを無効にする
  if (nowStatus == STATUS_MQTT_SUC) return;

  if (btnPtn == BTN_OFF && btn) {
    btnPtn = BTN_DOWN;
    return;
  }

  if (btnPtn == BTN_DOWN && !btn) {
    btnPtn = BTN_OFF;
    nowStatus = STATUS_BTN_ON;
    return;
  }

}


void loopMQTT() {
  //送信中の待ち
  if (nowStatus == STATUS_MQTT_SUC) {
    unsigned long _t = millis();
    if ( abs(_t - waitingTime) >= WAIT_MQTT_PUBLISH) {
      nowStatus = STATUS_TAIKI;
    }
    return;
  }

  //ボタンを押した直後
  if (nowStatus == STATUS_BTN_ON) {

    //どの面が上なのか分からない
    if (jikiPtn == JIKI_NONE) {
      nowStatus = STATUS_TAIKI;
      return;
    }

    //接続できていない。
    if (!client.connected()) {
      nowStatus = STATUS_TAIKI;
      return;
    }

    publish();
    nowStatus = STATUS_MQTT_SUC;
    waitingTime = millis();
  }
}


void loop() {
  //Sleep
  interval<LOOPTIME_SLEEP_CNT>::run([] {
    if (++sleepCnt >= SLEEP_CNT_S) {
      NefryDisplay.clear();
      NefryDisplay.setFont(ArialMT_Plain_24);
      NefryDisplay.drawString(20, 5, "SLEEP....");
      NefryDisplay.setFont(ArialMT_Plain_10);
      NefryDisplay.drawString(20, 55, "...Please push RESET BTN");
      NefryDisplay.display();
      Nefry.sleep(-1);
    }
  });

  //JikiSensor
  interval<LOOPTIME_JIKI>::run([] {
    loopJikiSensor();
  });

  //Button
  interval<LOOPTIME_BTN>::run([] {
    loopBtn();
  });

  //MQTT publish
  interval<LOOPTIME_MQTT>::run([] {
    if (!client.connected()) reconnect();
    loopMQTT();
  });

  //Display
  interval<LOOPTIME_DISP>::run([] {
    loopDisplay();
  });

}
