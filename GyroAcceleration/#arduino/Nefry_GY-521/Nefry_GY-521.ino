//GY-521(MPU-6050搭載)の制御
//MPU-6050 Accelerometer + Gyro
#include <Nefry.h>

#include "GY521.h"

gy521 _gy521;

const unsigned int LOOP_TIME_US = 10000;  //ループ関数の周期(μsec)
int processingTime; //loopの頭から最後までの処理時間

void setup(){
  _gy521.Init();
}

void loop(){
  processingTime = micros();
  
  _gy521.GetData();  

  Nefry.println(_gy521.gyro_x);

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

