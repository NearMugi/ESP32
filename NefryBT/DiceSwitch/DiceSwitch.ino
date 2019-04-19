//箱型スイッチ
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
#define NEFRY_DATASTORE_BEEBOTTE_DICE 0
#define BBT "mqtt.beebotte.com"
#define QoS 0
String bbt_token;
#define Channel "Dice"
#define Res "act"
char topic[64];
WiFiClient espClient;
PubSubClient client(espClient);
String msg;
String tmpSendData = "[@1,@2]";
#define WAIT_NEXTTIME_MS 5000 //publishからの待ち時間(ms)
unsigned long waitTime; //一度publishしてからの待ち時間
bool isAct = false;
#define MSG_ACCEPT "Accepting actions..."
#define MSG_WAIT "Wait Next Time..."

//date
#include <time.h>
#define JST     3600*9

//mpu6050
#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;
#include "MPU6050_Action.h"
MPU6050_Action mpu_action;
bool isCalibration; //Calibration ON/OFF
int CalOfs[4] = { -263, -36, -13, 1149}; //Gyro x,y,z, Accel z
float mpu6050_Gravity[3];     //[x,y,z] どの面が上になっているか判断する
int mpu6050_WorldAccel[3];       //[x,y,z] アクションを判断する

//NefryDisplayMessage
String MsgMqtt;
String MsgAction;
String MsgMpu6050;
String ipStr; //ipアドレス

//ループ周期(us)
#include <interval.h>
#define LOOPTIME_MPU6050 10000

//connect mqtt broker
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE_DICE);
  const char* tmp = bbt_token.c_str();
  // Attempt to connect
  if (client.connect(clientId.c_str(), tmp, "")) {
    Serial.println("connected");
    MsgMqtt = "Mqtt Connected";
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    MsgMqtt = "Mqtt DisConnected";
  }
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, ipStr);
  NefryDisplay.drawString(0, 10, MsgMqtt);
  NefryDisplay.drawString(0, 20, MsgMpu6050);
  NefryDisplay.drawString(0, 30, msg);
  NefryDisplay.drawString(0, 40, MsgAction);

  NefryDisplay.display();
  Nefry.ndelay(10);
}

void setup() {
  Nefry.setProgramName("DiceSwitch");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);

  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  //mqtt
  client.setServer(BBT, 1883);
  sprintf(topic, "%s/%s", Channel, Res);
  Nefry.setStoreTitle("Dice_Token", NEFRY_DATASTORE_BEEBOTTE_DICE);

  //mpu6050
  //キャリブレーションする必要ない場合は指定したオフセットを渡す
  isCalibration = false;
  mpu_main.init(isCalibration, CalOfs);

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  //displayMessage
  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

  //上面・アクションの判定用変数の初期化
  mpu_action.initJdgAct();
  MsgAction = MSG_ACCEPT;
}

void loop() {
  if (!client.connected()) reconnect();

  if (isAct) {
    //次のPublish送信が有効になるまで待ち
    if ((millis() - waitTime) > WAIT_NEXTTIME_MS) {
      //再開
      mpu_action.initJdgAct();
      MsgAction = MSG_ACCEPT;
      isAct = false;
    } else {
      return;
    }
  }

  //mpu6050のデータを解析・アクションがあれば送信
  interval<LOOPTIME_MPU6050>::run([] {
    mpu_main.updateValue();
    mpu_main.Get_Gravity(mpu6050_Gravity);
    mpu_main.Get_WorldAccel(mpu6050_WorldAccel);
    MsgMpu6050 = mpu_main.GetMsg();

    mpu_action.Jdg_Top(mpu6050_Gravity);  //どの面が上になっているか判断
    mpu_action.Jdg_Action(mpu6050_WorldAccel); //アクションを判断
    //メッセージの作成
    msg = tmpSendData;
    msg.replace("@1", String(mpu_action.top));
    msg.replace("@2", String(mpu_action.action));

    //アクションがあった時だけ送信する
    if (mpu_action.action != ACT_KEEP) {
      if (client.connected()) {
        client.loop();
        publish();

        //待ちに切り替える
        MsgAction = MSG_WAIT;
        isAct = true;
        waitTime = millis();
      }
    }
  });
}

void publish()
{
  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();

  root["data"] = msg;
  root["ispublic"] = true;
  root["ts"] = time(NULL);

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer, QoS);
}

