#include <Nefry.h>
#include <NefryDisplay.h>
#include <Wire.h>

#include "ArduCAM.h"
#include <SPI.h>
#include "memorysaver.h"

const int CS = D5;
const int CAM_POWER_ON = D6;
ArduCAM myCAM(OV2640, CS);

#define NEFRY_DATASTORE_DRIVE_TOKEN 4
const char* URL = "https://www.googleapis.com/upload/drive/v3/files?uploadType=media";
const char* HOST = "www.googleapis.com";
String auth = "";

static const size_t bufferSize = 2048;
static uint8_t buffer[bufferSize] = {0xFF};
uint8_t temp = 0, temp_last = 0;
int i = 0;
bool is_header = false;

void Capture() {
  delay(1000);

  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

  SendCapture();
}

void SendCapture() {
  Serial.println(F("SendCapture"));

  uint32_t len  = myCAM.read_fifo_length();
  if (len >= MAX_FIFO_SIZE) //0x5FFFF      //384KByte
  {
    Serial.println(F("Over size."));
    len = MAX_FIFO_SIZE;
  }
  if (len == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  WiFiClient client;
reconnect:
  Serial.println(F("Try to connect.."));
  if (!client.connect(HOST, 80)) {
    client.stop();
    delay(1000);
    goto reconnect;
  }
  Serial.print(F("connect host:"));
  Serial.println(HOST);

  Serial.print(F("Content-Length: "));
  Serial.println(len);

  client.print(String("POST ") + URL + F(" HTTP/1.1\n") +
               F("Host: ") + HOST + F("\n") +
               F("Content-Type: text/plain\n") +
               F("Content-Length: ") + "4" + F("\n") +
               F("Authorization: Bearer ") + auth + F("\n") +
               F("Connection: close\n\n"));

  Serial.println(F("HTTP Sending..... "));

  client.write("HOGE");
  delay(1000);

  if (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(F("Responce: ")); Serial.println(line);
  }
  client.stop();

  Serial.println("Picture sent");
}



void setup() {
  Nefry.setProgramName("ArduCAM OV2640 Sample");

  Nefry.enableSW();

  Nefry.setStoreTitleStr("Drive Token", NEFRY_DATASTORE_DRIVE_TOKEN);
  auth = Nefry.getStoreStr(NEFRY_DATASTORE_DRIVE_TOKEN);

  uint8_t vid, pid;
  uint8_t temp;
  //set the CS as an output:
  pinMode(CS, OUTPUT);
  pinMode(CAM_POWER_ON , OUTPUT);
  digitalWrite(CAM_POWER_ON, HIGH);

  Wire.begin();

  Serial.println(F("ArduCAM Start!"));

  // initialize SPI:
  SPI.begin();
  SPI.setFrequency(4000000); //4MHz

  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("SPI1 interface Error!"));
    while (1);
  }

  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("SPI1 interface Error!"));
    while (1);
  }

  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
    Serial.println(F("Can't find OV2640 module!"));
  }  else {
    Serial.println(F("OV2640 detected."));
  }

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  myCAM.clear_fifo_flag();

}
void loop() {
  if (Nefry.readSW()) {
    Capture();
  }
}
