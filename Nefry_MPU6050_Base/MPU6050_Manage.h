#ifndef __MPU6050_MANAGE_H
#define __MPU6050_MANAGE_H

//#define OUTPUT_READABLE_YAWPITCHROLL
#define OUTPUT_READABLE_REALACCEL
//#define OUTPUT_READABLE_WORLDACCEL
#define OUTPUT_TEAPOT

class MPU6050_Manage {
  public:
    void init();
    void updateValue();
    void DebugPrint();

  private:
};

#endif
