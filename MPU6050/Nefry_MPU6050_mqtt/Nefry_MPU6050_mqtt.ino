#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

String ipStr;

#define NEFRY_DATASTORE_MOSQUITTO 0
#define NEFRY_DATASTORE_BEEBOTTE 1

#include <time.h>
#define JST     3600*9

#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;

//Calibration ON/OFF
bool isCalibration;

//MPU6050の初期化時に使用するオフセット
//CalibrationがOFFの時に適用される
int CalOfs[4] = {0, 0, 0, 0}; //Gyro x,y,z, Accel z

//MPU6050から取得するデータ
float mpu6050_Quaternion[4];  //[w,x,y,z]
String sendData_Quaternion = "@1,@2,@3,@4";

String mqtt_server;

#define BBT "mqtt.beebotte.com"
#define QoS 0
String bbt_token;
#define Channel "mpu6050"
#define RES_Q "Quaternion"
char topic[64];

WiFiClient espClient;
PubSubClient client(espClient);
String msg;

#define MAX_MSGSIZE 20
String getTopic;
char getPayload[MAX_MSGSIZE];

String MsgMqtt;
String MsgMpu6050;

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
  bbt_token = "token:";
  bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE);
  const char* tmp = bbt_token.c_str();
  // Attempt to connect
  if (client.connect(clientId.c_str(), tmp, "")) {
    Serial.println("connected");
    MsgMqtt = "Mqtt Connected";
    // Once connected, publish an announcement...
    //client.publish("comment", "Coming Packets");
    //client.subscribe("res");
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
  NefryDisplay.drawString(0, 0, ipStr);
  NefryDisplay.drawString(0, 10, MsgMqtt);
  NefryDisplay.drawString(0, 20, MsgMpu6050);
  NefryDisplay.drawString(0, 30, msg);

  NefryDisplay.display();
  Nefry.ndelay(10);
}

void setup() {
  Nefry.setProgramName("MPU6050 + BeeBotte");
  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();
  Nefry.ndelay(10);

  //キャリブレーションする必要ない場合は指定したオフセットを渡す
  isCalibration = false;
  CalOfs[0] = -263;
  CalOfs[1] = -36;
  CalOfs[2] = -13;
  CalOfs[3] = 1149;
  mpu_main.init(isCalibration, CalOfs);

  client.setServer(BBT, 1883);
  //client.setCallback(callback);

  // Create the topic to publish to
  sprintf(topic, "%s/%s", Channel, RES_Q);

  Nefry.setStoreTitle("MQTTServerIP", NEFRY_DATASTORE_MOSQUITTO); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", NEFRY_DATASTORE_BEEBOTTE);
  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}

void loop() {
  mpu_main.updateValue();
  mpu_main.Get_Quaternion(mpu6050_Quaternion);
  msg = sendData_Quaternion;
  msg.replace("@1", String(mpu6050_Quaternion[0]));
  msg.replace("@2", String(mpu6050_Quaternion[1]));
  msg.replace("@3", String(mpu6050_Quaternion[2]));
  msg.replace("@4", String(mpu6050_Quaternion[3]));
  MsgMpu6050 = mpu_main.GetErrMsg();

  if (!client.connected()) reconnect();
  if (client.connected()) {
    client.loop();
    publish(RES_Q, msg);
  }

  //delay(100);
}


void publish(const char* resource, String data)
{
  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();
  root["data"] = data;
  root["ispublic"] = true;
  root["ts"] = time(NULL);;

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer, QoS);
}

