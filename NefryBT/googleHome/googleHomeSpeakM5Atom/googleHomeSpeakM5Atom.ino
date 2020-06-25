// ++++++++++++++++++++++++++++
// M5Atomバージョン
// ++++++++++++++++++++++++++++

// MQTTで受信したメッセージをGoogleHomeで再生する
// GoogleHomeで再生する方法は以下を参照した
// https://github.com/nori-dev-akg/esp32-google-home-notifier-voicetext/blob/master/esp32-google-home-notifier-voicetext.ino
//
// MQTTで受信できるデータ数が限られているので、分割したデータを受信する
// メッセージ開始のキーワードを受信したら初期化、終了のキーワードを受信したらGoogleHomeで再生する

#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "WebServer.h"

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp8266-google-home-notifier.h>
#include "mqttConfig.h"
#include "secretConfig.h"

WebServer webServer(80);

// MQTT
const char *host = "mqtt.beebotte.com";
const char *clientId;
String bbt_token;
WiFiClientSecure espClient;
PubSubClient mqttClient(host, 8883, espClient);

#define MAX_MSGSIZE 100
String getTopic;
char getPayload[MAX_MSGSIZE];
String getWord;

// VoiceText Web API
const String tts_url = "https://api.voicetext.jp";
String tts_user = "";
const String tts_pass = "";
const String mp3file = "/tmp.mp3";
String tts_parms = "&speaker=hikari&volume=200&speed=100&format=mp3";

// GoogleHome
String startChar;
String endChar;
String sendMessage;
bool sendTrigger;
GoogleHomeNotifier ghn;
char displayName[] = "";
String googleHomeIPStr; //ipアドレス

String ipStr; //ipアドレス
#define JST 3600 * 9
time_t getTs;

bool reconnect()
{
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    const char *user = bbt_token.c_str();
    if (mqttClient.connect(clientId, user, NULL))
    {
        Serial.println("connected");
        mqttClient.subscribe(topicGoogleHome);
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
    getTopic = (String)topic;
    for (int i = 0; i < MAX_MSGSIZE; i++)
    {
        getPayload[i] = '\0';
    }
    for (int i = 0; i < length; i++)
    {
        getPayload[i] = (char)payload[i];
    }
    //Serial.println(getTopic);
    Serial.println((String)getPayload);

    // データを抜き出す
    StaticJsonDocument<200> root;
    DeserializationError error = deserializeJson(root, getPayload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    getWord = root["data"].as<String>();
    getTs = time(NULL);
    //Serial.println(getWord);
    //Serial.println((String)ctime(&getTs));

    sendTrigger = false;
    if (getWord == startChar)
    {
        Serial.println("Message Start");
        sendMessage = "";
    }
    else if (getWord == endChar)
    {
        Serial.println("Message End");
        sendTrigger = true;
    }
    else
    {
        sendMessage += getWord;
    }
}

// text to speech
void text2speech(String msg)
{
    // VoiceTextAPIにPOST
    String tmpURL = "api.voicetext.jp";
    const char *host = tmpURL.c_str();
    const int httpsPort = 443;

    String postData = String("text=") + URLEncode(msg.c_str()) + tts_parms;
    String auth = base64::encode(tts_user + ":" + tts_pass);

    String postHeader = "";
    postHeader += "POST /v1/tts?@postData HTTP/1.1\r\n";
    postHeader += "Host: @host:@httpsPort\r\n";
    postHeader += "Authorization: Basic @auth\r\n";
    postHeader += "Connection: close\r\n";
    postHeader += "Content-Type: application/x-www-form-urlencoded\r\n";
    postHeader += "\r\n";

    postHeader.replace("@host", String(host));
    postHeader.replace("@httpsPort", String(httpsPort));
    postHeader.replace("@auth", auth);
    postHeader.replace("@postData", postData);

    WiFiClientSecure clientVoiceText;
    Serial.print(F("Connecting to: "));
    Serial.print(host);
    if (!clientVoiceText.connect(host, httpsPort))
    {
        Serial.println(F(" Failed..."));
        return;
    }
    Serial.println(F(" Success!"));

    // POST
    clientVoiceText.print(postHeader);
    clientVoiceText.flush();

    delay(10);

    // SPIFFSにmp3ファイルを生成
    SPIFFS.begin();
    File fs = SPIFFS.open(mp3file, FILE_WRITE);

    unsigned long contentLength = 0;
    while (clientVoiceText.connected())
    {
        String line = clientVoiceText.readStringUntil('\n');
        if (line.indexOf("Content-Length: ") >= 0)
        {
            String tmp = line.substring(line.indexOf(":") + 1);
            contentLength = tmp.toInt();
        }
        Serial.print(line);

        if (line == "\r")
        {
            Serial.println(F("headers received"));
            //Serial.println(clientVoiceText.available());

            // データを読み取る
            Serial.print(F("Read Data Size : "));
            Serial.println(contentLength);
            //※clientVoiceText.available()>0で判定すると全てのデータを取得できない。
            //  一度ゼロになって、またデータを取得するような動きになる。
            //　そのためデータ数分(Content-Length)取得する処理にする。
            time_t startTime = time(NULL);
            int limitTime = 0;
            while (contentLength > 0 && limitTime < 10)
            {
                limitTime = (unsigned long)(time(NULL) - startTime);
                if (clientVoiceText.available())
                {
                    char c = clientVoiceText.read();
                    fs.print(c);
                    contentLength--;
                }
            }
            Serial.print(F("Read Time : "));
            Serial.println(limitTime);
            break;
        }
    }
    clientVoiceText.stop();
    fs.close();

    fs = SPIFFS.open(mp3file, FILE_READ);
    Serial.print(F("mp3 Data Size : "));
    Serial.println(fs.size());
    fs.close();

    // GoogleHomeに送信
    String mp3url = "http://" + ipStr + mp3file;
    Serial.println(F("GoogleHome start"));
    Serial.println(mp3url.c_str());
    if (ghn.play(mp3url.c_str()) != true)
    {
        Serial.println(F("failed..."));
        Serial.println(ghn.getLastError());
    }
    Serial.println(F("Success!"));

    SPIFFS.end();
    Serial.println(F("client stop"));
}

// from http://hardwarefun.com/tutorials/url-encoding-in-arduino
// modified by chaeplin
String URLEncode(const char *msg)
{
    const char *hex = "0123456789ABCDEF";
    String encodedMsg = "";

    while (*msg != '\0')
    {
        if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') ||
            ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~')
        {
            encodedMsg += *msg;
        }
        else
        {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 0xf];
        }
        msg++;
    }
    return encodedMsg;
}

void handlePlay()
{
    SPIFFS.begin(true);
    File file = SPIFFS.open(mp3file, FILE_READ);
    Serial.println("handlePlay: sending " + mp3file + ":" + file.size());
    webServer.streamFile(file, "audio/mp3");
    file.close();
    SPIFFS.end();
}

void setup()
{
    //Wifi
    Serial.print(F("Wifi "));
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.println(F(" Connect"));

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    //displayMessage
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    // MQTT
    espClient.setCACert(beebottle_ca_cert);
    uint64_t chipid = ESP.getEfuseMac();
    String tmp = "ESP32-" + String((uint16_t)(chipid >> 32), HEX);
    clientId = tmp.c_str();
    bbt_token = "token:" + beebotteToken;
    mqttClient.setCallback(callback);

    // VoiceText API
    tts_user = voiceTextAPIKey;
    webServer.on(mp3file.c_str(), handlePlay);
    webServer.begin();

    // GoogleHome
    googleHomeName.toCharArray(displayName, googleHomeName.length() + 1);
    startChar = MQTTStartChar;
    endChar = MQTTEndChar;
    sendMessage = "";
    sendTrigger = false;

    Serial.print(F("connecting to Google Home["));
    Serial.print(displayName);
    Serial.println(F("]..."));

    if (ghn.device(displayName, "ja") != true)
    {
        Serial.println(ghn.getLastError());
        return;
    }
    IPAddress tmpIP = ghn.getIPAddress();
    googleHomeIPStr = String(tmpIP[0]) + '.' + String(tmpIP[1]) + '.' + String(tmpIP[2]) + '.' + String(tmpIP[3]);
    Serial.print("found Google Home(");
    Serial.print(googleHomeIPStr);
    Serial.print(":");
    Serial.print(ghn.getPort());
    Serial.println(")");

    // Debug
    //sendMessage = "これはテストです。";
    //sendTrigger = true;
}

void loop()
{
    // MQTT Clientへ接続
    if (!mqttClient.connected())
    {
        reconnect();
    }
    else
    {
        mqttClient.loop();
    }

    if (sendTrigger)
    {
        Serial.println("Message Send To GoogleHome");
        Serial.println(sendMessage);
        text2speech(sendMessage);

        sendMessage = "";
        sendTrigger = false;
    }
}
