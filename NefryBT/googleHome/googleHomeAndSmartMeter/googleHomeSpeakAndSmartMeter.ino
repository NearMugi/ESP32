// MQTTで受信したメッセージをGoogleHomeで再生する
// GoogleHomeで再生する方法は以下を参照した
// https://github.com/nori-dev-akg/esp32-google-home-notifier-voicetext/blob/master/esp32-google-home-notifier-voicetext.ino
//
// MQTTで受信できるデータ数が限られているので、分割したデータを受信する
// メッセージ開始のキーワードを受信したら初期化、終了のキーワードを受信したらGoogleHomeで再生する
// 
// スマートメーターの値を取得してMQTTを送信する

#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp8266-google-home-notifier.h>
#include "mqttConfig.h"
#include "intervalMs.h"
#include "bp35a1.h"

#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

// Nefry Environment data
#define beebotteTokenIdx 0
#define beebotteTokenTag "Beebotte_Token"

#define googleHomeIPIdx 1
#define googleHomeIPTag "GoogleHome_IP"

#define googleHomeRoomIdx 2
#define googleHomeRoomTag "GoogleHome_RoomName"

#define voiceTextAPIKeyIdx 3
#define voiceTextAPIKeyTag "VoiceTextAPI_Key"

#define mqttStartCharIdx 4
#define mqttStartCharTag "MQTT_StartChar"

#define mqttEndCharIdx 5
#define mqttEndCharTag "MQTT_EndChar"

#define beebotteSmartMeterIdx 6
#define beebotteSmartMeterTag "Beebotte SmartMeter Token"
#define smartMeterIDIdx 7
#define smartMeterIDTag "Smart Meter ID"
#define smartMeterPWIdx 8
#define smartMeterPWTag "Smart Meter Password"

// ループ周期(ms)

// 即時電力値・電流値・累積電力値を取得
#define LOOPTIME_GET_EP_VALUE 60 * 5 * 1000
// 接続確認
#define LOOPTIME_CHECK_CONNECT 60 * 5 * 1000

// MQTT
const char *host = "mqtt.beebotte.com";
int QoS = 1;
const char *clientId;
String bbt_token;
WiFiClientSecure espClient;
PubSubClient mqttClient(host, 8883, espClient);
// MQTT(SmartMeter)
const char *clientIdSmartMeter;
String bbt_tokenSmartMeter;
WiFiClientSecure espClientSmartMeter;
PubSubClient mqttClientSmartMeter(host, 8883, espClientSmartMeter);

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

// NefryDisplayMessage
String msgIsConnect;
String ipStr; //ipアドレス
#define JST 3600 * 9
time_t getTs;

// SmartMeter
bp35a1 bp;

bool reconnect()
{
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    const char *user = bbt_token.c_str();
    if (mqttClient.connect(clientId, user, NULL))
    {
        Serial.println("connected");
        msgIsConnect = "MQTT Connected";
        mqttClient.subscribe(topic);
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
        msgIsConnect = "MQTT DisConnected";
    }
    return mqttClient.connected();
}

bool reconnectSmartMeter()
{
    Serial.print("Attempting MQTT(SmartMeter) connection...");
    // Attempt to connect
    const char *user = bbt_tokenSmartMeter.c_str();
    if (mqttClientSmartMeter.connect(clientIdSmartMeter, user, NULL))
    {
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClientSmartMeter.state());
    }
    return mqttClientSmartMeter.connected();
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

    Nefry.setLed(0, 0, 255);

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
    Nefry.setLed(0, 0, 0);
}

void handlePlay()
{
    SPIFFS.begin(true);
    File file = SPIFFS.open(mp3file, FILE_READ);
    Serial.println("handlePlay: sending " + mp3file + ":" + file.size());
    Nefry.getWebServer()->ESP32WebServer::streamFile(file, "audio/mp3");
    file.close();
    SPIFFS.end();
}

void DispNefryDisplay()
{
    NefryDisplay.clear();

    String tmpGoogleHomeIP = "GoogleHome:" + googleHomeIPStr;
    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, ipStr);
    NefryDisplay.drawString(0, 12, tmpGoogleHomeIP);
    NefryDisplay.drawString(0, 24, msgIsConnect);
    NefryDisplay.drawString(0, 36, (String)ctime(&getTs));

    NefryDisplay.display();
    Nefry.ndelay(10);
}

void setup()
{
    Nefry.setProgramName("GoogleHome speaks MQTT Msg And SmartMeter Reading");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);
    Nefry.setStoreTitle(googleHomeIPTag, googleHomeIPIdx);
    Nefry.setStoreTitle(googleHomeRoomTag, googleHomeRoomIdx);
    Nefry.setStoreTitle(voiceTextAPIKeyTag, voiceTextAPIKeyIdx);
    Nefry.setStoreTitle(mqttStartCharTag, mqttStartCharIdx);
    Nefry.setStoreTitle(mqttEndCharTag, mqttEndCharIdx);
    Nefry.setStoreTitle(beebotteSmartMeterTag, beebotteSmartMeterIdx);
    Nefry.setStoreTitle(smartMeterIDTag, smartMeterIDIdx);
    Nefry.setStoreTitle(smartMeterPWTag, smartMeterPWIdx);    
    Nefry.setLed(0, 0, 0);

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    //displayMessage
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
    NefryDisplay.autoScrollFunc(DispNefryDisplay);

    // MQTT
    espClient.setCACert(beebottle_ca_cert);
    uint64_t chipid = ESP.getEfuseMac();
    String tmp = "ESP32GoogleHome-" + String((uint16_t)(chipid >> 32), HEX);
    clientId = tmp.c_str();
    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_token = "token:";
    bbt_token += Nefry.getStoreStr(beebotteTokenIdx);
    mqttClient.setCallback(callback);

    // MQTT(SmartMeter)
    espClientSmartMeter.setCACert(beebottle_ca_cert);
    uint64_t chipidSmartMeter = ESP.getEfuseMac();
    tmp = "ESP32SmartMeter-" + String((uint16_t)(chipidSmartMeter >> 32), HEX);
    clientIdSmartMeter = tmp.c_str();
    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_tokenSmartMeter = "token:";
    bbt_tokenSmartMeter += Nefry.getStoreStr(beebotteSmartMeterIdx);

    // VoiceText API
    tts_user = Nefry.getStoreStr(voiceTextAPIKeyIdx);
    Nefry.getWebServer()->on(mp3file.c_str(), handlePlay);

    // GoogleHome
    String tmpRoom = Nefry.getStoreStr(googleHomeRoomIdx);
    tmpRoom.toCharArray(displayName, tmpRoom.length() + 1);
    startChar = Nefry.getStoreStr(mqttStartCharIdx);
    endChar = Nefry.getStoreStr(mqttEndCharIdx);
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

    // SmartMeter
    // シリアル通信するポート(RX:D2=23, TX:D3=19);
    int PIN_RX = 23;
    int PIN_TX = 19;
    String serviceID = Nefry.getStoreStr(smartMeterIDIdx);
    String servicePW = Nefry.getStoreStr(smartMeterPWIdx);
    bp.init(PIN_RX, PIN_TX, serviceID, servicePW);
    bp.connect();

    // Debug
    //sendMessage = "これはテストです。";
    //sendTrigger = true;
}

void loop()
{
    bp.connect();

    // MQTT Clientへ接続
    if (!mqttClient.connected())
    {
        reconnect();
    }
    else
    {
        mqttClient.loop();
    }

    // MQTT Client(SmartMeter)へ接続
    if (!mqttClientSmartMeter.connected())
    {
        reconnectSmartMeter();
    }
    else
    {
        mqttClientSmartMeter.loop();
    }

    if (sendTrigger)
    {
        Serial.println("Message Send To GoogleHome");
        Serial.println(sendMessage);
        text2speech(sendMessage);

        sendMessage = "";
        sendTrigger = false;
    }

    interval<LOOPTIME_CHECK_CONNECT>::run([] {
        bp.chkConnect();
    });

    interval<LOOPTIME_GET_EP_VALUE>::run([] {
        bp.getEPValue();

        if (bp.epA > 0.0f)
        {
            //日付を取得する
            time_t t = time(NULL);
            struct tm *tm;
            tm = localtime(&t);
            char getDate[15] = "";
            sprintf(getDate, "%04d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

            // mqtt送信向けにJsonデータを生成する
            char bufferData[200];
            StaticJsonDocument<200> root;
            root["date"] = getDate;
            root["a"] = bp.epA;
            root["kw"] = bp.epkW;
            root["ts"] = t;
            serializeJson(root, bufferData);
            Serial.println(bufferData);

            char bufferTotal[200];
            StaticJsonDocument<200> rootTotal;
            rootTotal["date"] = bp.date;
            rootTotal["tkw"] = bp.totalkWh;
            rootTotal["ts"] = t;
            serializeJson(rootTotal, bufferTotal);
            Serial.println(bufferTotal);

            if (mqttClientSmartMeter.connected())
            {
                mqttClientSmartMeter.publish(topicData, bufferData, QoS);
                mqttClientSmartMeter.publish(topicTotal, bufferTotal, QoS);
                Serial.println(F("MQTT(SmartMeter) publish!"));
            }
        }
    });
}
