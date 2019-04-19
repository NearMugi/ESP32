//Nefry(ESP32)でサーボモータを動かすサンプル
//使うサーボモータSG90のスペック
//20ms(50Hz)周期　On時間0.5ms～2.4msで-90～90度に対応
#include "esp_system.h"

#define SG90_PWM 50
#define SG90_MIN 26  // (26/1024)*20ms ≒ 0.5 ms  (-90°)
#define SG90_MAX 123 // (123/1024)*20ms ≒ 2.4 ms (+90°)

#define PIN_SERVO D6
#define LEDC_CH_SERVO 0
#define LEDC_BIT 10

int n;
#define RANGE_MIN 0
#define RANGE_MAX 100
bool rev;

const unsigned int LOOP_TIME_US = 50000;  //ループ関数の周期(μsec)
int processingTime; //loopの頭から最後までの処理時間


unsigned int GetServoPWM(unsigned int _v) {
  long rtn = map(_v, RANGE_MIN, RANGE_MAX, SG90_MIN, SG90_MAX);
  return (unsigned int)rtn;
}

void setup() {
  Serial.begin(115200);

  ledcSetup(LEDC_CH_SERVO, SG90_PWM, LEDC_BIT); //使用チャンネル・周期・分解能
  ledcAttachPin(PIN_SERVO, LEDC_CH_SERVO);  //ピンとチャンネルを紐づける
  n = (RANGE_MIN + RANGE_MAX) / 2;  //原点スタート
  rev = true;
}

void loop() {

  processingTime = micros();


  if (rev) {
    n++;
    if (n > RANGE_MAX) {
      n = RANGE_MAX;
      rev = !rev;
    }
  } else {
    n--;
    if (n < RANGE_MIN) {
      n = RANGE_MIN;
      rev = !rev;
    }
  }
  unsigned int _pwm =   GetServoPWM(n);
  Serial.println(_pwm);
  ledcWrite(LEDC_CH_SERVO, _pwm);

  //一連の処理にかかった時間を考慮して待ち時間を設定する
  wait_ConstantLoop();

}

void wait_ConstantLoop() {
  processingTime = micros() - processingTime;
  long loopWaitTime = LOOP_TIME_US - processingTime;

  if (loopWaitTime < 0)  return;

  long start_time = micros();
  while ( micros() - start_time < loopWaitTime) {};
}

