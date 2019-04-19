#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>

String mqtt_server;

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
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("msg/comment", "Coming Packets");
      client.subscribe("msg/led");
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
  //NefryのDataStoreに書き込んだIPアドレス(String)を(const char*)に変換
  Nefry.setStoreTitle("MQTTServerIP", 0);
  mqtt_server = Nefry.getStoreStr(0);
  Serial.print("MQTTServer IP:");
  Serial.println(mqtt_server);
  const char* tmp = mqtt_server.c_str();
  client.setServer(tmp, 1883);
  client.setCallback(callback);
  
  Nefry.setProgramName("MQTT Server Submit Test");
  NefryDisplay.autoScrollFunc(DisplayPrint);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void DisplayPrint(){
  NefryDisplay.drawString(0, 0, "Submit msg");
  NefryDisplay.drawString(0, 20, getTopic);
  NefryDisplay.drawString(0, 40, (String)getPayload);
}

