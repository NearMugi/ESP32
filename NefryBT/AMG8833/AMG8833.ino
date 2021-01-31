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

#include "intervalMs.h"
#include "env.h"

//date
#include <time.h>
#define JST 3600 * 9

const int PIXEL_ARRAY_SIZE = 8 * 8;
float pixels[PIXEL_ARRAY_SIZE];

Adafruit_AMG88xx amg;
bool status;

// Post SpreadSheet
WiFiClientSecure client;
String header = "";

// LoopTime
const int LOOPTIME_POST = 5 * 1000;

void setup()
{

    NefryDisplay.begin();

    //date
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    NefryDisplay.clear();
    NefryDisplay.display();

    status = amg.begin(0x69);

    header = "POST " + url + " HTTP/1.1\r\n";
    header += "Host: " + String(host) + ":443\r\n";
    header += "Content-Type: application/json\r\n";
    header += "Connection: Keep-Alive\r\n";
}

void loop()
{
    interval<LOOPTIME_POST>::run([] {
        amg.readPixels(pixels);

        String data = "";
        int j = 0;
        int k = 0;
        for (int i = 0; i < PIXEL_ARRAY_SIZE; i++)
        {
            data += String(pixels[i]);
            data += ",";
        }

        Serial.println(data.length());
        Serial.println(data);

        if (client.connect(host, 443))
        {
            String json = "{\"data\":\"" + data + "\"}";

            client.print(header);
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
    });
}