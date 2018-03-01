#include "Arduino.h"
#include "GY521.h"

//コンストラクタ
gy521::gy521() {}

void gy521::Init() {
  Wire.begin();
  uint8_t c;
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  Serial.print("WHO_AM_I : ");
  Serial.print(c, HEX);
  Serial.print(", error = ");
  Serial.println(error, DEC);
  error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);
  Serial.print("PWR_MGMT_1 : ");
  Serial.print(c, HEX);
  Serial.print(", error = ");
  Serial.println(error, DEC);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

void gy521::Getdt() {
  dT = ( (float) accel_t_gyro.value.temperature + 12412.0) / 340.0;
}
void gy521::GetAccGyro() {
  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));
  uint8_t swap;
#define SWAP(x,y) swap = x; x = y; y = swap
  SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
  SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
  SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
  SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
  SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
  SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);

  acc_x = accel_t_gyro.value.x_accel / 16384.0; //FS_SEL_0 16,384 LSB / g
  acc_y = accel_t_gyro.value.y_accel / 16384.0;
  acc_z = accel_t_gyro.value.z_accel / 16384.0;

  acc_angle_x = atan2(acc_x, acc_z) * 360 / 2.0 / PI;
  acc_angle_y = atan2(acc_y, acc_z) * 360 / 2.0 / PI;
  acc_angle_z = atan2(acc_x, acc_y) * 360 / 2.0 / PI;

  gyro_x = accel_t_gyro.value.x_gyro / 131.0;  //FS_SEL_0 131 LSB / (°/s)
  gyro_y = accel_t_gyro.value.y_gyro / 131.0;
  gyro_z = accel_t_gyro.value.z_gyro / 131.0;
}

// MPU6050_read
int gy521::MPU6050_read(int start, uint8_t *buffer, int size) {
  Serial.println("MPU6050_read");
  int i, n, error;
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);
  n = Wire.endTransmission(false);   // hold the I2C-bus
  if (n != 0)
    return (n);
  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size);
  i = 0;
  while (Wire.available() && i < size) {
    buffer[i++] = Wire.read();
  }
  if ( i != size)
    return (-11);
  return (0);  // return : no error
}

// MPU6050_write
int gy521::MPU6050_write(int start, const uint8_t *pData, int size) {
  int n, error;
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);
  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);
  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);
  return (0);         // return : no error
}

// MPU6050_write_reg
int gy521::MPU6050_write_reg(int reg, uint8_t data) {
  int error;
  error = MPU6050_write(reg, &data, 1);
  return (error);
}
