#ifndef __MPU6050_MANAGE_H
#define __MPU6050_MANAGE_H

class MPU6050_Manage {
  public:
    void reset();
    void init(bool _isCalibration, int _ofs[4]);
    void updateValue();
    void Get_EulerAngle(float v[3]);
    void Get_Quaternion(float v[4]);
    void Get_RealAccel(int v[3]);
    void Get_WorldAccel(int v[3]);
    void Get_teapotPacket(uint8_t v[14]);
    String GetErrMsg();
    
    int CalOfs[4];  //Gyro x,y,z, Accel z
    bool isFinishInitialize;
  private:
};

#endif
