#include <Nefry.h>
#include <NefryDisplay.h>
#include <Adafruit_AMG88xx.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

//date
#include <time.h>
#define JST     3600*9

//MQTT
#define NEFRY_DATASTORE_BEEBOTTE 1
#define BBT "mqtt.beebotte.com"
#define QoS 0
String bbt_token;
#define Channel "AMG8833"
#define Res0 "a0"
#define Res1 "a1"
#define Res2 "a2"
#define Res3 "a3"
#define Res4 "a4"
#define Res5 "a5"
#define Res6 "a6"
#define Res7 "a7"
char topic[8][15];
WiFiClient espClient;
PubSubClient client(espClient);

const int pixel_array_size = 8*8;
float pixels[pixel_array_size];
const unsigned int dataSize = 100;

Adafruit_AMG88xx amg;
bool status;

void mqttPublish(int _idx, String _data, time_t _t){    
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_token = "token:";
    bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE);
    const char* tmp = bbt_token.c_str();
    // Attempt to connect
    if (client.connect(clientId.c_str(), tmp, "")) {
        char buffer[dataSize];
        StaticJsonBuffer<dataSize> jsonOutBuffer;
        JsonObject& root = jsonOutBuffer.createObject();
        root["data"] = _data;
        root["ts"] = _t;

        // Now print the JSON into a char buffer
        root.printTo(buffer, sizeof(buffer));
        
        // Now publish the char buffer to Beebotte
        client.publish(topic[_idx], buffer, QoS);
    }    
}

void setup() {

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
  
    //date
    configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    
    Nefry.setStoreTitle("MQTT_Token", NEFRY_DATASTORE_BEEBOTTE); 
    client.setServer(BBT, 1883);
    
    sprintf(topic[0], "%s/%s", Channel, Res0);
    sprintf(topic[1], "%s/%s", Channel, Res1);
    sprintf(topic[2], "%s/%s", Channel, Res2);
    sprintf(topic[3], "%s/%s", Channel, Res3);
    sprintf(topic[4], "%s/%s", Channel, Res4);
    sprintf(topic[5], "%s/%s", Channel, Res5);
    sprintf(topic[6], "%s/%s", Channel, Res6);
    sprintf(topic[7], "%s/%s", Channel, Res7);

    NefryDisplay.clear();
    NefryDisplay.display();

    status = amg.begin(0x69);
}

void loop() {
    if(!status) {
        Serial.println("Connect Error...");
        return;
    }

    amg.readPixels(pixels);

    time_t  t = time(NULL);
    String data = "";
    int j = 0;
    int k = 0;
    for(int i=0; i<pixel_array_size; i++){
        data += String(pixels[i]);
        data += ",";

        if(++j >= 8){
            mqttPublish(k++, data, t);
            j = 0;
            data = "";
        }
    }

    delay(100);
}