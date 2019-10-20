#include <M5Stack.h>

int sensor = 26;

void setup()
{
    pinMode(sensor, INPUT);
    m5.begin();
}

void loop()
{
    M5.Lcd.fillScreen(0x0000); //画面を真っ黒にする（これがないと文字に文字が重なって数字が読めない）
    int value = analogRead(sensor);
    M5.Lcd.setTextSize(16);   //文字の大きさ
    M5.Lcd.setCursor(70, 48); //文字の位置
    M5.Lcd.print(value);      //表示する文字
    M5.update();
    delay(200);
}