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
#define QoS 1
String bbt_token;
#define Channel "Dice"
#define Res "act"
char topic[64];
WiFiClient espClient;
PubSubClient client(espClient);
String msg;
String tmpSendData = "[@1,@2]";
#define WAIT_NEXTTIME_MS 5000 //publishからの待ち時間(ms)
uint16_t waitTime; //一度publishしてからの待ち時間
bool sw = false;

//date
#include <time.h>
#define JST     3600*9

//mpu6050
#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;
bool isCalibration; //Calibration ON/OFF
int CalOfs[4] = { -263, -36, -13, 1149}; //Gyro x,y,z, Accel z
float mpu6050_Gravity[3];     //[x,y,z] どの面が上になっているか判断する
int mpu6050_WorldAccel[3];       //[x,y,z] アクションを判断する

//上面(1～6)
#define Z_P 1
#define Z_M 6
#define X_P 2
#define X_M 5
#define Y_P 3
#define Y_M 4
int top;

//アクション
#define ACT_KEEP 0    //変化なし
#define ACT_SHAKE_V 1 //垂直方向に振る
#define ACT_SHAKE_H 2 //水平方向に降る
#define ACT_KNOCK 3   //ノックする
int action;

#define MAX_IDX 150
int idx;
int accel_max[3]; //区間内での最大値
int accel_abs[3][MAX_IDX]; //加速度(絶対値)

//NefryDisplayMessage
String MsgMqtt;
String MsgSw;
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
  NefryDisplay.drawString(0, 40, MsgSw);

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
  initJdgAct();
}

void loop() {
  if (!client.connected()) reconnect();

  if (sw) {
    //次のPublish送信が有効になるまで待ち
    if ((millis() - waitTime) > WAIT_NEXTTIME_MS) {
      //再開
      initJdgAct();
    } else {
      return;
    }
  }

  //mpu6050のデータを解析・アクションがあれば送信
  interval<LOOPTIME_MPU6050>::run([] {
    mpu_main.updateValue();
    MsgMpu6050 = mpu_main.GetMsg();

    Jdg_Top();  //どの面が上になっているか判断
    Jdg_Action(); //アクションを判断

    //メッセージの作成
    msg = tmpSendData;
    msg.replace("@1", String(top));
    msg.replace("@2", String(action));

    //アクションがあった時だけ送信する
    if (action != ACT_KEEP) {
      if (client.connected()) {
        client.loop();
        publish();

        //待ちに切り替える
        MsgSw = "Wait Next Time...";
        sw = true;
        waitTime = millis();
      }
    }
  });
}

void initJdgAct() {
  sw = false;
  MsgSw = "Accepting actions...";
  top = Z_P;
  action = ACT_KEEP;
  idx = 0;
  accel_max[0] = 0;
  accel_max[1] = 0;
  accel_max[2] = 0;
  for (int i = 0; i < MAX_IDX; i++) {
    accel_abs[0][i] = 0;
    accel_abs[1][i] = 0;
    accel_abs[2][i] = 0;
  }
}

void Jdg_Top() {
  //絶対値が一番大きい座標を上の面とする。
  //その座標の２面のうちどちらが上になっているかは正負で判断する。
  mpu_main.Get_Gravity(mpu6050_Gravity);
  int abs_x = abs(mpu6050_Gravity[0] * (float)100);
  int abs_y = abs(mpu6050_Gravity[1] * (float)100);
  int abs_z = abs(mpu6050_Gravity[2] * (float)100);

  if (abs_x > abs_y && abs_x > abs_z) {
    if (mpu6050_Gravity[0] >= 0) {
      top = X_P;
    } else {
      top = X_M;
    }
  }

  if (abs_y > abs_x && abs_y > abs_z) {
    if (mpu6050_Gravity[1] >= 0) {
      top = Y_P;
    } else {
      top = Y_M;
    }
  }

  if (abs_z >= abs_x && abs_z >= abs_y) {
    if (mpu6050_Gravity[2] >= 0) {
      top = Z_P;
    } else {
      top = Z_M;
    }
  }


}

#define BORDER_LINE_SHAKE 2500  //一度でもこの値を超えたとき"振った"と判断する
#define BORDER_LINE_BOTTOM 200  //山の下限(下がったところ)
#define MOUNTAIN_CNT 4  //振ったと判断する山の数
#define BORDER_LINE_KEEP 100    //この値より小さいときは"維持かも"と判断する
#define KEEP_CNT 10 //BORDER_LINE_KEEPをKEEP_CNT回連続して下回ると"維持"と判断する
void Jdg_Action() {
  //加速度の変化量を基に振っているかどうか判断する。
  action = ACT_KEEP;

  //加速度の絶対値と最大値を保存
  mpu_main.Get_WorldAccel(mpu6050_WorldAccel);
  for (int i = 0; i < 3; i++) {
    accel_abs[i][idx] = abs(mpu6050_WorldAccel[i]);
    if (accel_abs[i][idx] > accel_max[i]) accel_max[i] = accel_abs[i][idx];
  }
  if (++idx >= MAX_IDX) {
    idx = MAX_IDX - 1;
    //前に詰める
    for (int i = 0; i < MAX_IDX - 2; i++) {
      accel_abs[0][i] = accel_abs[0][i + 1];
      accel_abs[1][i] = accel_abs[1][i + 1];
      accel_abs[2][i] = accel_abs[2][i + 1];
    }
  }

  //解析
  //連続した山の数が指定個以上
  //一度でもボーダーラインを超える
  //3軸の中で一番高い山
  //上記を満たす場合、その方向に振ったと判断する。
  bool isclear[3];//条件を満たした。
  int mountain[3]; //山の数
  int tmp_max[3];  //山の中で一番高い値
  int keep_cnt; //連続して下回った回数
  bool isJdgStart; //山かも

  for (int i = 0; i < 3; i++) {
    isclear[i] = false;
    mountain[i] = 0;
    tmp_max[i] = 0;
    //一度もボーダーラインを超えていない場合は何もしない。
    if (accel_max[i] < BORDER_LINE_SHAKE) {
      continue;
    }

    isJdgStart = false;
    keep_cnt = 0;
    //山を数える
    for (int j = 0; j < idx; j++) {
      if (isclear[i]) break;

      if (!isJdgStart) {
        if (accel_abs[i][j] > BORDER_LINE_KEEP) {
          isJdgStart = true;
          keep_cnt = 0;
        } else {
          keep_cnt++;
          //"維持"が一定以上続いた場合は全てリセット
          if (keep_cnt >= KEEP_CNT) {
            isJdgStart = false;
            keep_cnt = KEEP_CNT;
            mountain[i] = 0;
            tmp_max[i] = 0;
          }
        }
      }

      if (isJdgStart) {
        if (accel_abs[i][j] > tmp_max[i]) tmp_max[i] = accel_abs[i][j];
        //下限に達したとき、山であると判断する
        if (accel_abs[i][j] < BORDER_LINE_BOTTOM) {
          mountain[i]++;
          isJdgStart = false; //次の山を見つける

          if (mountain[i] >= MOUNTAIN_CNT) isclear[i] = true;
        }
      }
      
    }
  }

  //3軸を比較してアクションを判定する
  //x,y方向はどちらも"横振り"と判断するので、xまたはyとzを比較する
  bool isHorizon = false;
  if (isclear[0] || isclear[1]) {
    action = ACT_SHAKE_H;
    isHorizon = true;
  }
  if (isclear[2]) {
    if(isHorizon){
      if(tmp_max[2] > tmp_max[0] && tmp_max[2] > tmp_max[1]){
         action = ACT_SHAKE_V;
      }
    } else {
      action = ACT_SHAKE_V;
    }
  }
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

