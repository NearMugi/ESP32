#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include "MPU6050_Calibration.h"

void MPU6050_Calibration::init(MPU6050 _accelgyro) {
  accelgyro = _accelgyro;
  buffersize = 1000;
  acel_deadzone = 8;
  giro_deadzone = 1;
  state = 0;
  accelgyro.setXAccelOffset(0);
  accelgyro.setYAccelOffset(0);
  accelgyro.setZAccelOffset(0);
  accelgyro.setXGyroOffset(0);
  accelgyro.setYGyroOffset(0);
  accelgyro.setZGyroOffset(0);
}

int MPU6050_Calibration::GetOfs_GyroX() {
  return gx_offset;
}
int MPU6050_Calibration::GetOfs_GyroY() {
  return gy_offset;
}
int MPU6050_Calibration::GetOfs_GyroZ() {
  return gz_offset;
}
int MPU6050_Calibration::GetOfs_AcelZ() {
  return az_offset;
}

bool MPU6050_Calibration::main() {
  bool isEnd = false;

  if (state == 0) {
    Serial.println("Reading sensors for first time...");
    meansensors();
    state++;
    delay(1000);
  }

  if (state == 1) {
    Serial.println("Calculating offsets...");
    calibration();
    state++;
    delay(1000);
  }

  if (state == 2) {
    meansensors();
    isEnd = true;
    Serial.println("FINISHED!");
#if true
    Serial.print("Sensor readings with offsets:\t");
    String msg = String(mean_ax) + "\t";
    msg += String(mean_ay) + "\t";
    msg += String(mean_az) + "\t";
    msg += String(mean_gx) + "\t";
    msg += String(mean_gy) + "\t";
    msg += String(mean_gz) + "\n";
    msg += "Your offsets:\t";
    msg += String(ax_offset) + "\t";
    msg += String(ay_offset) + "\t";
    msg += String(az_offset) + "\t";
    msg += String(gx_offset) + "\t";
    msg += String(gy_offset) + "\t";
    msg += String(gz_offset) + "\t";
    Serial.println(msg);
    Serial.println("\nData is printed as: acelX acelY acelZ giroX giroY giroZ");
    Serial.println("Check that your sensor readings are close to 0 0 16384 0 0 0");
    Serial.println("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");

    Serial.println("");
#endif
  }


  return isEnd;

}


void MPU6050_Calibration::meansensors() {
  long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

  while (i < (buffersize + 101)) {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (i > 100 && i <= (buffersize + 100)) { //First 100 measures are discarded
      buff_ax = buff_ax + ax;
      buff_ay = buff_ay + ay;
      buff_az = buff_az + az;
      buff_gx = buff_gx + gx;
      buff_gy = buff_gy + gy;
      buff_gz = buff_gz + gz;
    }
    if (i == (buffersize + 100)) {
      mean_ax = buff_ax / buffersize;
      mean_ay = buff_ay / buffersize;
      mean_az = buff_az / buffersize;
      mean_gx = buff_gx / buffersize;
      mean_gy = buff_gy / buffersize;
      mean_gz = buff_gz / buffersize;
    }
    i++;
    delay(2); //Needed so we don't get repeated measures
  }
}

void MPU6050_Calibration::calibration() {
  ax_offset = -mean_ax / 8;
  ay_offset = -mean_ay / 8;
  az_offset = (16384 - mean_az) / 8;

  gx_offset = -mean_gx / 4;
  gy_offset = -mean_gy / 4;
  gz_offset = -mean_gz / 4;
  while (1) {
    int ready = 0;
    accelgyro.setXAccelOffset(ax_offset);
    accelgyro.setYAccelOffset(ay_offset);
    accelgyro.setZAccelOffset(az_offset);

    accelgyro.setXGyroOffset(gx_offset);
    accelgyro.setYGyroOffset(gy_offset);
    accelgyro.setZGyroOffset(gz_offset);

    meansensors();
    Serial.println("...");

    if (abs(mean_ax) <= acel_deadzone) ready++;
    else ax_offset = ax_offset - mean_ax / acel_deadzone;

    if (abs(mean_ay) <= acel_deadzone) ready++;
    else ay_offset = ay_offset - mean_ay / acel_deadzone;

    if (abs(16384 - mean_az) <= acel_deadzone) ready++;
    else az_offset = az_offset + (16384 - mean_az) / acel_deadzone;

    if (abs(mean_gx) <= giro_deadzone) ready++;
    else gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

    if (abs(mean_gy) <= giro_deadzone) ready++;
    else gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

    if (abs(mean_gz) <= giro_deadzone) ready++;
    else gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

    if (ready == 6) break;
  }
}

