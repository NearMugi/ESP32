// Grove SoundSensor
#define PIN_ADC  A1

float ave_unit = 0;
float RC_a = 0.95;  //ローパスフィルタの係数
float ave_sec = 0;
float maxAnalogValue = 0;
int cnt = 0;
const int MAX_CNT = 50;
long ave_tmp = 0;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  ave_unit = getAnalogValue(ave_unit);
  if (ave_unit > maxAnalogValue ) maxAnalogValue = ave_unit;

  //1テンポ遅れて平均値を出している。
  ave_tmp += ave_unit;
  if (++cnt >= MAX_CNT) {
    ave_sec = ave_tmp / MAX_CNT;
    cnt = 0;
    ave_tmp = 0;
  }

  Serial.print(ave_unit);
  Serial.print(",");
  Serial.print(ave_sec);
  Serial.print(",");
  Serial.print(maxAnalogValue);
  Serial.println("");

  delay(10);
}

//RCフィルタ適用
long getAnalogValue(float y_old) {
  int x = analogRead(PIN_ADC);
  float y = RC_a * y_old + (1 - RC_a) * x;
  return y;
}


