static const int pin_accx = A2;  // accelaration X of KXR94-2050
static const int pin_accy = A3;  // accelaration Y of KXR94-2050
static const int pin_accz = A4;  // accelaration Z of KXR94-2050

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pin_accx, INPUT);
  pinMode(pin_accy, INPUT);
  pinMode(pin_accz, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("x:");
  Serial.print(analogRead(pin_accx));
  Serial.print("\t");
  Serial.print("y:");
  Serial.print(analogRead(pin_accy));
  Serial.print("\t");
  Serial.print("z:");
  Serial.print(analogRead(pin_accz));
  Serial.print("\r\n");
  delay(1);
}
