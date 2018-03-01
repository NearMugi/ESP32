//GY-521(MPU-6050搭載)の制御
//MPU-6050 Accelerometer + Gyro
#include <Nefry.h>
#include <Wire.h>

#include "GY521.h"

gy521 _gy521;

void setup(){
    Serial.println("setup");
  _gy521.Init();
    Serial.println("InitEnd");
}

void loop(){
  Serial.println("loop");
  _gy521.Getdt();
  _gy521.GetAccGyro();  
}

