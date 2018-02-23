
//ジャイロセンサーの値を取得してみる
const int PIN_GYRO_X = A0;  //G1
const int PIN_GYRO_Y = A1;  //G2

float gyroX_mean, gyroY_mean;  //センサー静止時の入力値

float Gyro_GetAngularVelocity(int inputData, float mean);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PIN_GYRO_X, INPUT);
  pinMode(PIN_GYRO_Y, INPUT);

  //平均値をセンサー静止時の入力値とする
  gyroX_mean = 0;
  gyroY_mean = 0;
  for (int i = 0; i < 1024; ++i) {
    gyroX_mean += analogRead(PIN_GYRO_X);
    gyroY_mean += analogRead(PIN_GYRO_Y);
  }
  gyroX_mean /= 1024.;
  gyroY_mean /= 1024.;

  Serial.print("gyroX_mean:");
  Serial.print(gyroX_mean);

  Serial.print(" gyroY_mean:");
  Serial.print(gyroY_mean);

  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  float x = analogRead(PIN_GYRO_X);
  float deg_x = Gyro_GetAngularVelocity(x,gyroX_mean);

  float y = analogRead(PIN_GYRO_Y);
  float deg_y = Gyro_GetAngularVelocity(y,gyroY_mean);

  #if true
  Serial.print("Deg_X:");
  Serial.print(deg_x);

  Serial.print(" Deg_Y:");
  Serial.print(deg_y);

  Serial.println("");
#endif
  delay(1);

  
}
//ジャイロセンサーから角速度を取得する
//http://arduinopid.web.fc2.com/c5.html
float Gyro_GetAngularVelocity(int inputData, float mean){
  float v;  //電圧値(V)
  float v_mean; //静止時の電圧値(V)
  float deg;

  v = (float)inputData / (1024-1) * 5;
  v_mean = mean / (1024-1) * 5;
  deg = (v - v_mean) / 0.00067;

  return deg;
}

