// ロードセルを使って重さを測定する
// サンプルコード http://akizukidenshi.com/catalog/g/gK-12370/
#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include "mqttConfig.h"
#include "intervalMs.h"
#include "loadCell.h"

void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

// ループ周期(ms)
#define LOOPTIME_READ 10 * 1000
#define LOOPTIME_SEND 60 * 5 * 1000

// Nefry Environment data
#define beebotteTokenIdx 0
#define beebotteTokenTag "Beebotte_Token"

// MQTT
const char *host = "mqtt.beebotte.com";
int QoS = 1;
const char *clientId;
String bbt_token;
WiFiClientSecure espClient;
PubSubClient mqttClient(host, 8883, espClient);

// LoadCell
loadCell lc;
float readLoadCell;
#define pin_dout 13    //D7
#define pin_slk 14     //D8
#define OUT_VOL 0.001f //定格出力 [V]
#define LOAD 2000.0f   //定格容量 [g]

// NefryDisplayMessage
String mqttIsConnect;
String ipStr; //ipアドレス
#define JST 3600 * 9
time_t sendTs;

bool reconnect()
{
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    const char *user = bbt_token.c_str();
    if (mqttClient.connect(clientId, user, NULL))
    {
        Serial.println("connected");
        mqttIsConnect = "OK";
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
        mqttIsConnect = "NG";
    }
    return mqttClient.connected();
}

void DispNefryDisplay()
{
    NefryDisplay.clear();
    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, ipStr);
    NefryDisplay.drawString(0, 17, "MQTT :" + mqttIsConnect);

    char s[20];
    dtostrf(readLoadCell, 5, 3, s);
    NefryDisplay.drawString(0, 34, "Weight :" + String(s));
    NefryDisplay.drawString(0, 51, (String)ctime(&sendTs));
    NefryDisplay.display();

    Nefry.ndelay(500);
}

void setup()
{
    Nefry.setProgramName("Send LoadCell Value to MQTT");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);
    Nefry.setLed(0, 0, 0);

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    sendTs = 0;
    //displayMessage
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
    NefryDisplay.autoScrollFunc(DispNefryDisplay);

    // MQTT
    espClient.setCACert(beebottle_ca_cert);
    uint64_t chipid = ESP.getEfuseMac();
    String tmp = "ESP32-" + String((uint16_t)(chipid >> 32), HEX);
    clientId = tmp.c_str();
    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_token = "token:";
    bbt_token += Nefry.getStoreStr(beebotteTokenIdx);

    lc.init(pin_dout, pin_slk, OUT_VOL, LOAD);
}

void loop()
{
    // MQTT Clientへ接続
    if (!mqttClient.connected())
    {
        reconnect();
    }

    // Read LoadCell
    interval<LOOPTIME_READ>::run([] {
        readLoadCell = lc.getData();
    });

    // Send
    interval<LOOPTIME_SEND>::run([] {
        Nefry.setLed(128, 128, 0);
        //日付を取得する
        sendTs = time(NULL);
        struct tm *tm;
        tm = localtime(&sendTs);
        char getDate[15] = "";
        sprintf(getDate, "%04d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        // mqtt送信向けにJsonデータを生成する
        char bufferData[200];
        StaticJsonDocument<200> root;
        root["date"] = getDate;
        root["weight"] = readLoadCell;
        serializeJson(root, bufferData);
        Serial.println(bufferData);

        if (mqttClient.connected())
        {
            mqttClient.publish(topicWeight, bufferData, QoS);
            Serial.println(F("MQTT(SmartMeter) publish!"));
            Nefry.setLed(0, 0, 0);
        }
    });
}