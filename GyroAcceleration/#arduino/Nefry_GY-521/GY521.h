#ifndef GY-521_H
#define GY-521_H
#include <Wire.h>

#define MPU6050_ACCEL_XOUT_H       0x3B   // R
#define MPU6050_WHO_AM_I           0x75   // R
#define MPU6050_PWR_MGMT_1         0x6B   // R/W
#define MPU6050_I2C_ADDRESS 0x68

typedef union accel_t_gyro_union {
  struct {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  }
  reg;
  struct {
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
    int16_t temperature;
    int16_t x_gyro;
    int16_t y_gyro;
    int16_t z_gyro;
  }
  value;
};

class gy521
{
  public:
    int error;
    float dT;
    float acc_x;
    float acc_y;
    float acc_z;
    float acc_angle_x;
    float acc_angle_y;
    float acc_angle_z;    
    float gyro_x;
    float gyro_y;
    float gyro_z;
    
    gy521();
    void Init();
    void GetData();
  private:
    accel_t_gyro_union accel_t_gyro;
    int MPU6050_read(int start, uint8_t *buffer, int size);
    int MPU6050_write(int start, const uint8_t *pData, int size);
    int MPU6050_write_reg(int reg, uint8_t data);

};
#endif
