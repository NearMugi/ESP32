#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>

String mqtt_server;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
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

  Nefry.setProgramName("MQTT Server Publish Test");
  NefryDisplay.autoScrollFunc(DisplayPrint);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    client.publish("msg/comment", msg);
  }
}

void DisplayPrint(){
  NefryDisplay.drawString(0, 0, "Publish msg");
  NefryDisplay.drawString(0, 20, (String)msg);
}

