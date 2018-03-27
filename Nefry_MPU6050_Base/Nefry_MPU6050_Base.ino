#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;

const unsigned int LOOP_TIME_US = 10000;  //ループ関数の周期(μsec)
int processingTime; //loopの頭から最後までの処理時間

void setup() {
  Serial.begin(115200);
  mpu_main.init();
}

void loop() {
  processingTime = micros();
  mpu_main.updateValue();
  mpu_main.DebugPrint();

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
