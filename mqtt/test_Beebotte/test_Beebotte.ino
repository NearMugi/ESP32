#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>

String mqtt_server;

#define BBT "mqtt.beebotte.com"
String bbt_token; 

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
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
      client.publish("comment", "Coming Packets");
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

void setup() {

  client.setServer(BBT, 1883);
  client.setCallback(callback);

  Nefry.setStoreTitle("MQTTServerIP", 0); //mosquitto用なので今回は使わない。
  Nefry.setStoreTitle("BeeBotte_Token", 1);
//  mqtt_server = Nefry.getStoreStr(0);
  
  Nefry.enableSW();
  Nefry.setProgramName("BeeBotte Publish Submit Test");
  NefryDisplay.autoScrollFunc(DisplayPrint);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Nefry.readSW()){
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    client.publish("comment", msg);    
  }
}

void DisplayPrint(){
  NefryDisplay.drawString(0, 0, "Publish msg");
  NefryDisplay.drawString(0, 15, (String)msg);
  
  NefryDisplay.drawString(0, 30, "Submit msg");
  NefryDisplay.drawString(0, 45, (String)getPayload);
}

