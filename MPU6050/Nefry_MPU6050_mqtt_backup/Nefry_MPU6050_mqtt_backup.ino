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
float mpu6050_Quaternion[4];  //[w,x,y,z]
String sendData_Quaternion = "@1,@2,@3,@4";

String mqtt_server;

#define BBT "mqtt.beebotte.com"
String bbt_token; 
#define Channel "mpu6050"
#define RES_Q "Quaternion"
char topic[64];
    
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
  NefryDisplay.drawString(0, 0, (String)msg);
  
  NefryDisplay.display();
  Nefry.ndelay(100);
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

  // Create the topic to publish to
  sprintf(topic, "%s/%s", Channel, RES_Q);
    
  Nefry.setStoreTitle("MQTTServerIP", 0); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", 1);
  NefryDisplay.autoScrollFunc(DispNefryDisplay);

}

void loop() {

  mpu_main.updateValue();
  mpu_main.Get_Quaternion(mpu6050_Quaternion);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  msg = sendData_Quaternion;
  msg.replace("@1", String(mpu6050_Quaternion[0]));
  msg.replace("@2", String(mpu6050_Quaternion[1]));
  msg.replace("@3", String(mpu6050_Quaternion[2]));
  msg.replace("@4", String(mpu6050_Quaternion[3]));
  
  publish(RES_Q, msg);

  delay(100);
}


void publish(const char* resource, String data)
{
    StaticJsonBuffer<128> jsonOutBuffer;
    JsonObject& root = jsonOutBuffer.createObject();
    root["data"] = data;
    root["ispublic"] = true;
    root["ts"] = 0; 

    // Now print the JSON into a char buffer
    char buffer[128];
    root.printTo(buffer, sizeof(buffer));

    // Now publish the char buffer to Beebotte
    client.publish(topic, buffer);
}

