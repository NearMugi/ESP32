//VCC -> 5v
//GND -> GND
//SCL -> A5
//SDA -> A4
#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <NefrySetting.h>
void setting(){
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
float mpu6050_EulerAngle[3];  //[x,y,z]
float mpu6050_Quaternion[4];  //[w,x,y,z]
int mpu6050_RealAccel[3];        //[x,y,z]
int mpu6050_WorldAccel[3];       //[x,y,z]
uint8_t mpu6050_teapotPacket[14];

const unsigned int LOOP_TIME_US = 20000;  //ループ関数の周期(μsec)
int processingTime; //loopの頭から最後までの処理時間

String mqtt_server;

#define BBT "mqtt.beebotte.com"
String bbt_token; 
#define Channel "mpu6050"
#define RES_Q "Quaternion"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
String msg;
int value = 0;

#define MAX_MSGSIZE 20
String getTopic;
char getPayload[MAX_MSGSIZE];

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
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
      // Once connected, publish an announcement...
      //client.publish("comment", "Coming Packets");
      client.subscribe("res");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
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
  NefryDisplay.drawString(0, 0, "Publish msg");
  NefryDisplay.drawString(0, 15, (String)msg);
  
  NefryDisplay.display();
  Nefry.ndelay(20);
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
  client.setCallback(callback);

  Nefry.setStoreTitle("MQTTServerIP", 0); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", 1);
//  mqtt_server = Nefry.getStoreStr(0);

}

void loop() {
  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  processingTime = micros();
  mpu_main.updateValue();
  mpu_main.Get_Quaternion(mpu6050_Quaternion);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  msg = String(mpu6050_Quaternion[0]);
  msg +=",";
  msg += String(mpu6050_Quaternion[1]);
  msg +=",";
  msg += String(mpu6050_Quaternion[2]);
  msg +=",";
  msg += String(mpu6050_Quaternion[3]);
  
  publish(RES_Q, msg);
  
  //一連の処理にかかった時間を考慮して待ち時間を設定する
  wait_ConstantLoop();
}

void wait_ConstantLoop() {
  processingTime = micros() - processingTime;
  long loopWaitTime = LOOP_TIME_US - processingTime;

  if (loopWaitTime < 0)  return;

  long start_time = micros();
  while ( micros() - start_time < loopWaitTime) {};
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

    // Create the topic to publish to
    char topic[64];
    sprintf(topic, "%s/%s", Channel, resource);

    // Now publish the char buffer to Beebotte
    client.publish(topic, buffer);
}

