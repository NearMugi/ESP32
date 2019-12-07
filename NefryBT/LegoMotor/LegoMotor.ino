#include <Nefry.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);


// Motor setting
#define PIN_MOTOR A0
#define PIN_MOTOR_RE A1
#define MOTOR_CH 0

#define DELAY_TIME 1000

void setup() {
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_MOTOR_RE, OUTPUT);
}

void loop() {
  digitalWrite(PIN_MOTOR, HIGH);
  digitalWrite(PIN_MOTOR_RE, LOW);
  delay(DELAY_TIME);

  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_MOTOR_RE, HIGH);
  delay(DELAY_TIME);
}