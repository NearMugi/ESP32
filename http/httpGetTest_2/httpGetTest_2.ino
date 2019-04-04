//以下を参考にしてみた
//https://github.com/dotstudio/nefrybt_camera_server
//http://okiraku-camera.tokyo/blog/?p=6008
#include <Nefry.h>
#include <WiFiClientSecure.h>


#define DELAY_TIME 5000
const char* host = "192.168.0.9"; //ここにサーバーホストを指定
const char* page = "/";

void setup() {
  Nefry.print(F("POST to "));
  Nefry.print(host);
  Nefry.println(page);

}

void loop() {
  getData();
  delay(DELAY_TIME);
}

void getData() {
  WiFiClient client;
reconnect:
  Nefry.setLed(255, 0, 0);
  Nefry.println("Try to connect..");
  if (!client.connect(host, 80)) {
    Nefry.setLed(0, 0, 0);
    client.stop();
    delay(1000);
    goto reconnect;
  }

  if (client.available()) {
    String line = client.readStringUntil('\r');
    Nefry.print(F("Responce: ")); Nefry.println(line);
  }
  client.stop();
}

