// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)

/* ============================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2012 Jeff Rowberg

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
*/
#include <Nefry.h>
#include <NefryDisplay.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#include "MPU6050_Manage.h"
#include "MPU6050_Calibration.h"
MPU6050 mpu;
MPU6050_Calibration cal;

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };

String ErrMsg;

void MPU6050_Manage::reset() {
  isFinishInitialize = false;
}

void MPU6050_Manage::init(bool _isCalibration, int _ofs[4]) {

  String text;
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);


  isFinishInitialize = false;
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // initialize device
  //Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  //Serial.println(F("Testing device connections..."));

  if (mpu.testConnection()) {
    text = F("MPU6050 connection successful");
  } else {
    text = F("MPU6050 connection failed");
  }
  Serial.println(text);
  NefryDisplay.drawString(0, 0, text);
  NefryDisplay.display();
  Nefry.ndelay(10);


  //Calibration
  if (_isCalibration) {
    Serial.println("\n[Start Calibration]");
    NefryDisplay.drawString(0, 30, F("Start Calibration..."));
    NefryDisplay.display();
    Nefry.ndelay(10);

    bool isEnd = false;
    cal.init(mpu);
    while (!isEnd) {
      isEnd = cal.main();
    }
    CalOfs[0] = cal.GetOfs_GyroX();
    CalOfs[1] = cal.GetOfs_GyroY();
    CalOfs[2] = cal.GetOfs_GyroZ();
    CalOfs[3] = cal.GetOfs_AcelZ();
    NefryDisplay.drawString(0, 40, F("End Calibration!"));
    NefryDisplay.display();
    Nefry.ndelay(10);

  } else {
    CalOfs[0] = _ofs[0];
    CalOfs[1] = _ofs[1];
    CalOfs[2] = _ofs[2];
    CalOfs[3] = _ofs[3];
  }
  // load and configure the DMP
  // Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  String msg = "[Set Offset]\t";
  msg += "Gyro X : " + String(CalOfs[0]) + "\t";
  msg += "Gyro Y : " + String(CalOfs[1]) + "\t";
  msg += "Gyro Z : " + String(CalOfs[2]) + "\t";
  msg += "Acel Z : " + String(CalOfs[3]);
  Serial.println(msg);
  mpu.setXGyroOffset(CalOfs[0]);
  mpu.setYGyroOffset(CalOfs[1]);
  mpu.setZGyroOffset(CalOfs[2]);
  mpu.setZAccelOffset(CalOfs[3]);


  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    //  Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();

    NefryDisplay.clear();
    NefryDisplay.drawString(0, 0, F("DMP ready!"));
    NefryDisplay.drawString(0, 10, F("packetSize:"));
    NefryDisplay.drawString(70, 10, String(packetSize));
    NefryDisplay.display();
    Nefry.ndelay(3000);

  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  isFinishInitialize = true;
}

void MPU6050_Manage::updateValue() {
  ErrMsg = "";
  // if programming failed, don't try to do anything
  if (!dmpReady) {
    ErrMsg = "DMP Not Ready";
    return;
  }

  //get INT_STATUS byte
  mpuIntStatus = mpu.getIntStatus();

  if (mpuIntStatus & 0x12)
  {
    // FIFOのデータサイズを取得
    const uint16_t fifoCount = mpu.getFIFOCount();
    if ((mpuIntStatus & 0x10) || (1024 <= fifoCount))
    {
      // オーバーフローを検出したらFIFOをリセット(そもそも検出されないコトあり…)
      mpu.resetFIFO();
      Serial.println(F("FIFO overflow!"));
      ErrMsg = F("FIFO overflow!");
    }
    else if ((mpuIntStatus & 0x02) && (packetSize <= fifoCount))
    {
      // データの読み出し可能
      // FIFOよりデータを読み出す
      mpu.getFIFOBytes(fifoBuffer, packetSize);
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      //mpu.dmpGetGravity(&gravity, &q);
      //mpu.dmpGetAccel(&aa, fifoBuffer);
      //mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
      //mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
      //mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    }
  }
}

void MPU6050_Manage::Get_EulerAngle(float v[3]) {
  v[0] = ypr[0] * 180 / M_PI;
  v[1] = ypr[1] * 180 / M_PI;
  v[2] = ypr[2] * 180 / M_PI;
}

void MPU6050_Manage::Get_Quaternion(float v[4]) {
  v[0] = q.w;
  v[1] = q.x;
  v[2] = q.y;
  v[3] = q.z;
}

void MPU6050_Manage::Get_RealAccel(int v[3]) {
  v[0] = aaReal.x;
  v[1] = aaReal.y;
  v[2] = aaReal.z;
}

void MPU6050_Manage::Get_WorldAccel(int v[3]) {
  v[0] = aaWorld.x;
  v[1] = aaWorld.y;
  v[2] = aaWorld.z;
}

void MPU6050_Manage::Get_teapotPacket(uint8_t v[14]) {

  teapotPacket[2] = fifoBuffer[0];
  teapotPacket[3] = fifoBuffer[1];
  teapotPacket[4] = fifoBuffer[4];
  teapotPacket[5] = fifoBuffer[5];
  teapotPacket[6] = fifoBuffer[8];
  teapotPacket[7] = fifoBuffer[9];
  teapotPacket[8] = fifoBuffer[12];
  teapotPacket[9] = fifoBuffer[13];
  teapotPacket[11]++; // packetCount, loops at 0xFF on purpose

  for (int i = 0; i < 14; i++) {
    v[i] = teapotPacket[i];
  }
}

String MPU6050_Manage::GetErrMsg() {
  return ErrMsg;
}

