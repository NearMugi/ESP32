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
#include <WiFiClientSecure.h>
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
        Serial.println(client.state());
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
    Nefry.setProgramName("GoogleHome speaks MQTT Msg");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);
    Nefry.setStoreTitle(googleHomeIPTag, googleHomeIPIdx);
    Nefry.setStoreTitle(googleHomeRoomTag, googleHomeRoomIdx);
    Nefry.setStoreTitle(voiceTextAPIKeyTag, voiceTextAPIKeyIdx);
    Nefry.setStoreTitle(mqttStartCharTag, mqttStartCharIdx);
    Nefry.setStoreTitle(mqttEndCharTag, mqttEndCharIdx);
    Nefry.setLed(0, 0, 0);

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

    // Debug
    //sendMessage = "これはテストです。";
    //sendTrigger = true;
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
