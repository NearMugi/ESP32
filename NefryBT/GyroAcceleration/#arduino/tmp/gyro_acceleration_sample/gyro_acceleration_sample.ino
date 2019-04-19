#include <Kalman.h> // Source: https://github.com/TKJElectronics/KalmanFilter

//ジャイロセンサーのPIN設定
static const int PIN_GYRO_X = A0;  // X AXIS ROTATION V of ENC-03R
static const int PIN_GYRO_Y = A1;  // Y AXIS ROTATION V of ENC-03R
//加速度センサーのPIN設定(x,yに注意)
static const int PIN_ACCY_X = A3;  // accelaration Y of KXR94-2050 (rotated left 90 degree on Breadboard)
static const int PIN_ACCY_Y = A2;  // accelaration X of KXR94-2050 (rotated left 90 degree on Breadboard)
//static const int PIN_ACCY_Z = A4;  // accelaration Z of KXR94-2050
static const int PIN_ACCY_Z = 34;  // accelaration Z of KXR94-2050

Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;

/* IMU Data */
//ジャイロセンサー
double gyroX, gyroY;
double gyroX_mean, gyroY_mean;  //平均値
double gyroXrate, gyroYrate;

//加速度センサー
int accX, accY, accZ;
double accX_mean, accY_mean, accZ_mean;  //平均値

//計算結果
double accXangle, accYangle; // Angle calculate using the accelerometer
double gyroXangle, gyroYangle; // Angle calculate using the gyro
double compAngleX, compAngleY; // Calculate the angle using a complementary filter
double kalAngleX, kalAngleY; // Calculate the angle using a Kalman filter

//時間
uint32_t bef_timer;
uint32_t now_timer;
double dt;

//accXangle,accYangleをセットする
void SetAccAngle() {
  accX = analogRead(PIN_ACCY_X) - accX_mean;
  accY = analogRead(PIN_ACCY_Y) - accY_mean;
  accZ = analogRead(PIN_ACCY_Z) - accZ_mean;
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accYangle = (atan2(accX, accZ) + PI) * RAD_TO_DEG;
  accXangle = (atan2(accY, accZ) + PI) * RAD_TO_DEG;
}

//gyroXangle,gyroYangleをセットする
void SetGyroAngle() {
  gyroX = analogRead(PIN_GYRO_X) - gyroX_mean;
  gyroY = analogRead(PIN_GYRO_Y) - gyroY_mean;

  gyroXrate = 1.3 * gyroX;
  gyroYrate = 1.3 * gyroY;
  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * dt;
}

void setup() {
  Serial.begin(115200);
  //PIN設定
  pinMode(PIN_GYRO_X, INPUT);
  pinMode(PIN_GYRO_Y, INPUT);
  pinMode(PIN_ACCY_X, INPUT);
  pinMode(PIN_ACCY_Y, INPUT);
  pinMode(PIN_ACCY_Z, INPUT);

  //平均値を求める
  //センサーを水平に置いた時、加速度センサーのx,y軸方向は水平(0g)、z軸方向は垂直(1g)となっている。
  //z軸方向は平均を取ったとしても、+1gの値になってしまうので、x,y軸方向の平均から算出する。
  gyroX_mean = 0;
  gyroY_mean = 0;
  accX_mean = 0;
  accY_mean = 0;
  accZ_mean = 0;
  for (int i = 0; i < 1024; ++i) {
    gyroX_mean += analogRead(PIN_GYRO_X);
    gyroY_mean += analogRead(PIN_GYRO_Y);
    accX_mean +=  analogRead(PIN_ACCY_X);
    accY_mean +=  analogRead(PIN_ACCY_Y);
  }
  gyroX_mean /= 1024.;
  gyroY_mean /= 1024.;
  accX_mean /= 1024.;
  accY_mean /= 1024.;
  accZ_mean = (accX_mean + accY_mean) / 2;


  /* Set kalman and gyro starting angle */
  SetAccAngle();

  kalmanX.setAngle(accXangle); // Set starting angle
  kalmanY.setAngle(accYangle);
  gyroXangle = accXangle;
  gyroYangle = accYangle;
  compAngleX = accXangle;
  compAngleY = accYangle;

  bef_timer = micros();
}



void loop() {
  now_timer = micros();
  dt = (now_timer - bef_timer) / 1000000.;

  /* Update all the values */
  
  SetAccAngle();
  SetGyroAngle();
  
  // Calculate the angle using a Complimentary filter
  compAngleX = (0.93 * (compAngleX + (gyroXrate * dt))) + (0.07 * accXangle);
  compAngleY = (0.93 * (compAngleY + (gyroYrate * dt))) + (0.07 * accYangle);
  
  // Calculate the angle using a Kalman filter
  kalAngleX = kalmanX.getAngle(accXangle, gyroXrate, dt);
  kalAngleY = kalmanY.getAngle(accYangle, gyroYrate, dt);

  bef_timer = now_timer;


  /* Print Data */
#if false
  Serial.print("x(Y):");
  Serial.print(analogRead(PIN_ACCY_X));
  Serial.print("\t");
  Serial.print("y(X):");
  Serial.print(analogRead(PIN_ACCY_Y));
  Serial.print("\t");
  Serial.print("z(Z):");
  Serial.print(analogRead(PIN_ACCY_Z));
  Serial.print("\t");

  Serial.print("\t");
#endif

#if true
//  Serial.print(accXangle);
//  Serial.print("\t");
//  Serial.print(gyroXangle);
//  Serial.print("\t");
//  Serial.print(compAngleX);
//  Serial.print("\t");
//  Serial.print(kalAngleX);
//  Serial.print("\t");

  Serial.print("\t");

//  Serial.print(accYangle);
//  Serial.print("\t");
//  Serial.print(gyroYangle);
//  Serial.print("\t");
//  Serial.print(compAngleY);
//  Serial.print("\t");
  Serial.print(kalAngleY);
//  Serial.print("\t");

  //Serial.print(temp);Serial.print("\t");
#endif
  Serial.print("\r\n");
  delay(1);
}
