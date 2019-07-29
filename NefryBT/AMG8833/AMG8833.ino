#include <Nefry.h>
#include <NefryDisplay.h>
#include <Adafruit_AMG88xx.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
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
#define Channel "Cat"
#define Res "AMG8833"
char topic[64];
WiFiClient espClient;
PubSubClient client(espClient);

const int pixel_array_size = 8*8;
float pixels[pixel_array_size];
const unsigned int dataSize = 6 * pixel_array_size;

Adafruit_AMG88xx amg;
bool status;

void mqttPublish(String _data){    
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_token = "token:";
    bbt_token += Nefry.getStoreStr(NEFRY_DATASTORE_BEEBOTTE);
    const char* tmp = bbt_token.c_str();
    // Attempt to connect
    if (client.connect(clientId.c_str(), tmp, "")) {
        time_t  t = time(NULL);
        char buffer[dataSize];
        StaticJsonBuffer<dataSize> jsonOutBuffer;
        JsonObject& root = jsonOutBuffer.createObject();
        root["AMG8833"] = _data;
        root["ispublic"] = true;
        root["ts"] = t;

        // Now print the JSON into a char buffer
        root.printTo(buffer, sizeof(buffer));

        // Now publish the char buffer to Beebotte
        client.publish(topic, buffer, QoS);
    }    
}

void setup() {

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
  
    //date
    configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    
    Nefry.setStoreTitle("MQTT_Token", NEFRY_DATASTORE_BEEBOTTE); 
    client.setServer(BBT, 1883);
    sprintf(topic, "%s/%s", Channel, Res);

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

    String t = "";
    for(int i=0; i<pixel_array_size; i++){
        t += String(pixels[i]);
        t += ",";
    }
    Serial.println(t);
    mqttPublish(t);

    delay(100);
}