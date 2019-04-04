//GY-521(MPU-6050搭載)の制御
//MPU-6050 Accelerometer + Gyro
#include <Nefry.h>
#include <NefryDisplay.h>

#include "GY521.h"

gy521 _gy521;

const unsigned int LOOP_TIME_US = 10000;  //ループ関数の周期(μsec)
int processingTime; //loopの頭から最後までの処理時間


void DispNefryDisplay() {
  NefryDisplay.clear();
  String text;
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  text = "[ACC]";
  text += String(_gy521.acc_x);
  text += ",";
  text += String(_gy521.acc_y);
  text += ",";
  text += String(_gy521.acc_z);
  NefryDisplay.drawString(0, 0, text);

  text = "[GYR]";
  text += String(_gy521.gyro_x);
  text += ",";
  text += String(_gy521.gyro_y);
  text += ",";
  text += String(_gy521.gyro_z);
  NefryDisplay.drawString(0, 10, text);


  text = "[ANG]";
  text += String(_gy521.acc_angle_x);
  text += ",";
  text += String(_gy521.acc_angle_y);
  text += ",";
  text += String(_gy521.acc_angle_z);
  NefryDisplay.drawString(0, 20, text);

  
  NefryDisplay.display();
  Nefry.ndelay(10);
}


void setup() {
  _gy521.Init();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効
  
}

void loop() {
  processingTime = micros();

  _gy521.GetData();

  NefryDisplay.autoScrollFunc(DispNefryDisplay);
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

