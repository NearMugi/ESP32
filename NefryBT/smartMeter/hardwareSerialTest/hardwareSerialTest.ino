#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

HardwareSerial uart(1);

void setup()
{
    // (speed, type, RX:D2=23, TX:D3=19);
    uart.begin(115200, SERIAL_8N1, 23, 19);
    Serial.print("\n\nstart.\n");
}

void loop()
{
    if (Serial.available())
    {
        String in = Serial.readString();
        Serial.println(">>> " + in);
        uart.println(in);
    }
    if (uart.available())
    {
        String out = uart.readString();
        Serial.println("<<< " + out);
    }
    yield();
}