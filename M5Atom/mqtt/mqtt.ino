#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include "intervalMs.h"

#include "env.h"
#include "m5AtomBase.h"

// LoopTime
const int LOOPTIME_MQTT = 1 * 1000;

#include "mqttConfig.h"
#include "mqttESP32.h"
// Subscribe
static void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    const int MAX_MSGSIZE = 100;
    char getPayload[MAX_MSGSIZE];
    Serial.println((String)topic);
    for (int i = 0; i < MAX_MSGSIZE; i++)
    {
        getPayload[i] = '\0';
    }
    for (int i = 0; i < length; i++)
    {
        getPayload[i] = (char)payload[i];
    }
    Serial.println((String)getPayload);
}
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
}

void loop()
{
    if (_mqtt.getMqttState() != 0)
    {
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
    }
    _mqtt.loop();

    interval<LOOPTIME_MQTT>::run([] {
        if (_mqtt.getMqttState() != 0)
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
        root["err"] = _mqtt.getMqttConnectErr();
        root["ispublic"] = false;
        root["ts"] = sendTs;
        serializeJson(root, bufferData);
        Serial.println(bufferData);

        _mqtt.publish(bufferData);
    });
}
