// MQTTで受信したメッセージをGoogleHomeで再生する
// GoogleHomeで再生する方法は以下を参照した
// https://github.com/nori-dev-akg/esp32-google-home-notifier-voicetext/blob/master/esp32-google-home-notifier-voicetext.ino
//
#include <Nefry.h>
#include <NefryDisplay.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

// Nefry Environment data
#define beebotteTokenIdx 0
#define beebotteTokenTag "BeeBotte_Token"

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

//NefryDisplayMessage
String msgIsConnect;
String ipStr; //ipアドレス

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
    Serial.println("callback");

    getTopic = (String)topic;
    for (int i = 0; i < MAX_MSGSIZE; i++)
    {
        getPayload[i] = '\0';
    }
    for (int i = 0; i < length; i++)
    {
        getPayload[i] = (char)payload[i];
    }
    Serial.println(getTopic);
    Serial.println((String)getPayload);
}

void DispNefryDisplay()
{
    NefryDisplay.clear();

    NefryDisplay.setFont(ArialMT_Plain_10);
    NefryDisplay.drawString(0, 0, ipStr);
    NefryDisplay.drawString(0, 15, msgIsConnect);
    NefryDisplay.drawString(0, 30, (String)getPayload);

    NefryDisplay.display();
    Nefry.ndelay(10);
}

void setup()
{
    Nefry.setProgramName("GoogleHome speaks MQTT Msg");
    Nefry.setStoreTitle(beebotteTokenTag, beebotteTokenIdx);

    //displayMessage
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効
    NefryDisplay.autoScrollFunc(DispNefryDisplay);

    client.setServer(BBT, 1883);
    client.setCallback(callback);
    sprintf(topic, "%s/%s", Channel, Res);
}

void loop()
{
    if (!client.connected())
        reconnect();

    client.loop();
}
