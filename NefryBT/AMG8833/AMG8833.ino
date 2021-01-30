#include <Nefry.h>
#include <NefryDisplay.h>
#include <Adafruit_AMG88xx.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

#include "env.h"

//date
#include <time.h>
#define JST 3600 * 9

const int pixel_array_size = 8 * 8;
float pixels[pixel_array_size];
const unsigned int dataSize = 100;

Adafruit_AMG88xx amg;
bool status;

WiFiClientSecure client;

void setup()
{

    NefryDisplay.begin();
    NefryDisplay.setAutoScrollFlg(true); //自動スクロールを有効

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    NefryDisplay.clear();
    NefryDisplay.display();

    status = amg.begin(0x69);
}

void loop()
{
    amg.readPixels(pixels);

    String data = "";
    int j = 0;
    int k = 0;
    for (int i = 0; i < pixel_array_size; i++)
    {
        data += String(pixels[i]);
        data += ",";
    }

    Serial.println(data.length());
    Serial.println(data);

    if (client.connect(host, 443))
    {
        String json = "{\"data\":\"" + data + "\"}";

        client.print("POST " + url + " HTTP/1.1\r\n");
        client.print("Host: " + String(host) + ":443\r\n");
        client.print("Content-Type: application/json\r\n");
        client.print("Connection: Keep-Alive\r\n");
        client.print("Content-Length: " + String(json.length()) + "\r\n");
        client.print("\r\n");
        client.print(json + "\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0)
        {
            if (millis() - timeout > 10000)
            {
                Serial.println(">>> Client Timeout !");
                client.stop();
                return;
            }
        }

        while (client.available())
        {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }

        Serial.println("closing connection");
        client.stop();
    }

    delay(10000);
}