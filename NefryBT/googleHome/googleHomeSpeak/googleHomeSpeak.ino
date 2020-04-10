// MQTTで受信したメッセージをGoogleHomeで再生する
// GoogleHomeで再生する方法は以下を参照した
// https://github.com/nori-dev-akg/esp32-google-home-notifier-voicetext/blob/master/esp32-google-home-notifier-voicetext.ino
//
// MQTTで受信できるデータ数が限られているので、分割したデータを受信する
// メッセージ開始のキーワードを受信したら初期化、終了のキーワードを受信したらGoogleHomeで再生する

#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <HTTPClient.h>
#include <base64.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp8266-google-home-notifier.h>

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

// MQTT
#define BBT "mqtt.beebotte.com"
String bbt_token;
#define Channel "GoogleHome"
#define Res "message"
char topic[64];

WiFiClient espClient;
PubSubClient client(espClient);

#define MAX_MSGSIZE 100
String getTopic;
char getPayload[MAX_MSGSIZE];
String getWord;

// VoiceText Web API
const String tts_url = "https://api.voicetext.jp/v1/tts";
String tts_user = "";
const String tts_pass = "";
const String mp3file = "test.mp3";
String tts_parms = "&speaker=hikari&volume=200&speed=120&format=mp3";

// GoogleHome
String startChar;
String endChar;
String sendMessage;
bool sendTrigger;
GoogleHomeNotifier ghn;
const char displayName[] = "ファミリー ルーム";
String googleHomeIPStr; //ipアドレス

//NefryDisplayMessage
String msgIsConnect;
String ipStr; //ipアドレス
#define JST 3600 * 9
time_t getTs;

void reconnect()
{
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    //NefryのDataStoreに書き込んだToken(String)を(const char*)に変換
    bbt_token = "token:";
    bbt_token += Nefry.getStoreStr(beebotteTokenIdx);
    const char *tmp = bbt_token.c_str();
    // Attempt to connect
    if (client.connect(clientId.c_str(), tmp, ""))
    {
        Serial.println("connected");
        msgIsConnect = "Mqtt Connected";
        client.subscribe(topic);
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        msgIsConnect = "Mqtt DisConnected";
    }
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
    Serial.println(getWord);
    Serial.println((String)ctime(&getTs));

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
void text2speech(String text)
{

    HTTPClient http; // Initialize the client library
    size_t size = 0; // available streaming data size

    http.begin(tts_url); //Specify the URL

    Serial.println();
    Serial.println("Starting connection to tts server...");

    //request header for VoiceText Web API
    String auth = base64::encode(tts_user + ":" + tts_pass);
    http.addHeader("Authorization", "Basic " + auth);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String request = String("text=") + URLEncode(text.c_str()) + tts_parms;
    http.addHeader("Content-Length", String(request.length()));

    SPIFFS.begin();
    File f = SPIFFS.open("/" + mp3file, FILE_WRITE);
    if (f)
    {
        //Make the request
        int httpCode = http.POST(request);
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                http.writeToStream(&f);

                String mp3url = "http://" + ipStr + "/" + mp3file;
                Serial.println("GoogleHomeNotifier.play() start");
                if (ghn.play(mp3url.c_str()) != true)
                {
                    Serial.print("GoogleHomeNotifier.play() error:");
                    Serial.println(ghn.getLastError());
                    return;
                }
            }
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        f.close();
    }
    else
    {
        Serial.println("SPIFFS open error!!");
    }
    http.end();
    SPIFFS.end();
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
    Nefry.setProgramName("GoogleHome speaks MQTT Msg");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);
    Nefry.setStoreTitle(googleHomeIPTag, googleHomeIPIdx);
    Nefry.setStoreTitle(googleHomeRoomTag, googleHomeRoomIdx);
    Nefry.setStoreTitle(voiceTextAPIKeyTag, voiceTextAPIKeyIdx);
    Nefry.setStoreTitle(mqttStartCharTag, mqttStartCharIdx);
    Nefry.setStoreTitle(mqttEndCharTag, mqttEndCharIdx);

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    //displayMessage
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
    NefryDisplay.autoScrollFunc(DispNefryDisplay);

    client.setServer(BBT, 1883);
    client.setCallback(callback);
    sprintf(topic, "%s/%s", Channel, Res);

    // VoiceText API
    tts_user = Nefry.getStoreStr(voiceTextAPIKeyIdx);

    // GoogleHome
    startChar = Nefry.getStoreStr(mqttStartCharIdx);
    endChar = Nefry.getStoreStr(mqttEndCharIdx);
    sendMessage = "";
    sendTrigger = false;

    Serial.println("connecting to Google Home...");
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
}

void loop()
{
    if (!client.connected())
        reconnect();

    if (sendTrigger)
    {
        Serial.println("Message Send To GoogleHome");
        Serial.println(sendMessage);
        text2speech(sendMessage);

        sendMessage = "";
        sendTrigger = false;
    }

    client.loop();
}
