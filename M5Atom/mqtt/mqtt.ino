#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "intervalMs.h"

#include "env.h"
#include "mqttConfig.h"

#include "m5AtomBase.h"

// MQTT
const int LOOPTIME_MQTT = 1 * 1000;
int mqttConnectErr = 0;
WiFiClientSecure espClient;
PubSubClient mqttClient(host, 8883, espClient);

bool reconnect()
{
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect(clientId, token, NULL))
    {
        Serial.println("connected");
        mqttClient.subscribe(topicSub);
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
    }
    return mqttClient.connected();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println((String)topic);
}

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
    // MQTT
    mqttConnectErr = 0;
    espClient.setCACert(beebottle_ca_cert);
    mqttClient.setCallback(callback);
}

void loop()
{
    if (!mqttClient.connected())
    {
        reconnect();
        mqttConnectErr++;
    }
    else
    {
        mqttConnectErr = 0;
    }
    mqttClient.loop();
    if (mqttConnectErr > 3)
    {
        ESP.restart();
    }

    interval<LOOPTIME_MQTT>::run([] {
        if (!mqttClient.connected())
            return;
        //日付を取得する
        time_t sendTs = time(NULL);
        struct tm *tm;
        tm = localtime(&sendTs);
        char getDate[15] = "";
        sprintf(getDate, "%04d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        // mqtt送信向けにJsonデータを生成する
        char bufferData[200];
        StaticJsonDocument<200> root;
        root["data"] = getDate;
        root["ispublic"] = false;
        root["ts"] = sendTs;
        serializeJson(root, bufferData);
        Serial.println(bufferData);

        mqttClient.publish(topicPub, bufferData);
    });
}
