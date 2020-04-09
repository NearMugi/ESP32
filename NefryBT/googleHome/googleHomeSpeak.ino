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

#define voiceTextAPIKeyIdx 2
#define voiceTextAPIKeyTag "VoiceTextAPI_Key"

#define mqttStartCharIdx 3
#define mqttStartCharTag "MQTT_StartChar"

#define mqttEndCharIdx 4
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

// GoogleHome
String startChar;
String endChar;
String sendMessage;
bool sendTrigger;

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

void DispNefryDisplay()
{
    NefryDisplay.clear();

    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, ipStr);
    NefryDisplay.drawString(0, 12, msgIsConnect);
    NefryDisplay.drawString(0, 24, "[Get Time]");
    NefryDisplay.drawString(0, 36, (String)ctime(&getTs));

    NefryDisplay.display();
    Nefry.ndelay(10);
}

void setup()
{
    Nefry.setProgramName("GoogleHome speaks MQTT Msg");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);
    Nefry.setStoreTitle(googleHomeIPTag, googleHomeIPIdx);
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

    // GoogleHome
    startChar = Nefry.getStoreStr(mqttStartCharIdx);
    endChar = Nefry.getStoreStr(mqttEndCharIdx);
    sendMessage = "";
    sendTrigger = false;
}

void loop()
{
    if (!client.connected())
        reconnect();

    if (sendTrigger)
    {
        Serial.println("Message Send To GoogleHome");
        Serial.println(sendMessage);
        sendMessage = "";
        sendTrigger = false;
    }

    client.loop();
}
