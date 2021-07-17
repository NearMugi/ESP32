#include <Arduino.h>
#include "env.h"
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include "intervalMs.h"

#include <memory>
#include "m5AtomBase.h"

m5AtomBase m5Atom;

// LoopTime
const unsigned long LOOPTIME_MQTT = 5 * 1000;

#include "mqttConfig.h"
#include "mqttESP32.h"
// Subscribe
static void mqttCallback(char *topic, byte *payload, unsigned int length) {}
mqttESP32 _mqtt(
    host,
    topicPub, topicSub,
    clientId, token, beebottle_ca_cert,
    &mqttCallback);

void setup()
{
    M5.begin(true, false, true);

    Serial.begin(115200);
    WiFi.begin(ssid, pw);

    // wait for WiFi connection
    Serial.print("Waiting for WiFi to connect...");
    while ((WiFi.status() != WL_CONNECTED))
    {
        delay(100);
        Serial.print(".");
    }
    Serial.print("\nconnected ");
    Serial.println(WiFi.localIP());

    configTime(3600 * 9, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    m5Atom.LED.setWhite();
}

void loop()
{
    if (_mqtt.getMqttState() != 0)
    {
        m5Atom.LED.setRed();
        Serial.println("Attempting MQTT connection...");
        _mqtt.chkConnect();
        Serial.print("State(0->OK) : ");
        Serial.println(_mqtt.getMqttState());
        Serial.print("ConnectErr Count : ");
        Serial.println(_mqtt.getMqttConnectErr());
        if (_mqtt.getMqttConnectErr() > 3)
        {
            ESP.restart();
        }
        delay(1000);
    }
    _mqtt.loop();
    m5Atom.LED.setBlue();
    interval<LOOPTIME_MQTT>::run([]
                                 {
                                     m5Atom.LED.setBlue();
                                     if (_mqtt.getMqttState() != 0)
                                         return;

                                     // mqtt送信向けにJsonデータを生成する
                                     char bufferData[200];
                                     StaticJsonDocument<200> root;
                                     //root["ispublic"] = true;
                                     root["ts"] = time(NULL);
                                     serializeJson(root, bufferData);
                                     _mqtt.publish(bufferData);

                                     Serial.println(bufferData);
                                     m5Atom.LED.setGreen();
                                 });
    M5.update();
}
