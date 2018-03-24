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

#include "Nefry_I2Cdev.h"
#include "Nefry_MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
#include "MPU6050_Manage.h"
Nefry_MPU6050 mpu;

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

#include "MPU6050_Calibration.h"
MPU6050_Calibration cal;

void MPU6050_Manage::init() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  

  //Calibration
  Serial.println("\n[Start Calibration]");
  bool isEnd = false;
  cal.init(mpu);
  while (!isEnd) {
    isEnd = cal.main();
  }

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  String msg = "[Set Offset]\t";
  msg += "Gyro X : " + String(cal.GetOfs_GyroX()) +"\t";
  msg += "Gyro Y : " + String(cal.GetOfs_GyroY()) +"\t";
  msg += "Gyro Z : " + String(cal.GetOfs_GyroZ()) +"\t";
  msg += "Acel Z : " + String(cal.GetOfs_AcelZ());
  Serial.println(msg);
  mpu.setXGyroOffset(cal.GetOfs_GyroX());
  mpu.setYGyroOffset(cal.GetOfs_GyroY());
  mpu.setZGyroOffset(cal.GetOfs_GyroZ());
  mpu.setZAccelOffset(cal.GetOfs_AcelZ());

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void MPU6050_Manage::updateValue() {
  // if programming failed, don't try to do anything
  if (!dmpReady) return;

  //get INT_STATUS byte
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;


    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  }
}

void MPU6050_Manage::DebugPrint() {
#ifdef OUTPUT_READABLE_YAWPITCHROLL
  // display Euler angles in degrees
  Serial.print("ypr,");
  Serial.print(ypr[0] * 180 / M_PI);
  Serial.print(",");
  Serial.print(ypr[1] * 180 / M_PI);
  Serial.print(",");
  Serial.println(ypr[2] * 180 / M_PI);
#endif

#ifdef OUTPUT_READABLE_REALACCEL
  // display real acceleration, adjusted to remove gravity
  Serial.print("areal,");
  Serial.print(aaReal.x);
  Serial.print(",");
  Serial.print(aaReal.y);
  Serial.print(",");
  Serial.println(aaReal.z);
#endif

#ifdef OUTPUT_READABLE_WORLDACCEL
  // display initial world-frame acceleration, adjusted to remove gravity
  // and rotated based on known orientation from quaternion
  Serial.print("aworld,");
  Serial.print(aaWorld.x);
  Serial.print(",");
  Serial.print(aaWorld.y);
  Serial.print(",");
  Serial.println(aaWorld.z);
#endif

#ifdef OUTPUT_TEAPOT
  // display quaternion values in InvenSense Teapot demo format:
  teapotPacket[2] = fifoBuffer[0];
  teapotPacket[3] = fifoBuffer[1];
  teapotPacket[4] = fifoBuffer[4];
  teapotPacket[5] = fifoBuffer[5];
  teapotPacket[6] = fifoBuffer[8];
  teapotPacket[7] = fifoBuffer[9];
  teapotPacket[8] = fifoBuffer[12];
  teapotPacket[9] = fifoBuffer[13];
  Serial.write(teapotPacket, 14);
  teapotPacket[11]++; // packetCount, loops at 0xFF on purpose
#endif
}

