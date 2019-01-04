#include <Nefry.h>
#include <NefryDisplay.h>
#include <Wire.h>
#include <WiFiClientSecure.h>

#include "googleAPI.h"
googleAPI api;
#define NEFRY_DATASTORE_REFRESH_TOKEN 5
#define NEFRY_DATASTORE_CLIENT_ID 6
#define NEFRY_DATASTORE_CLIENT_SECRET 7
String refresh_token = "";
String client_id = "";
String client_secret = "";
String accessToken = "";

#include "ArduCAM.h"
#include <SPI.h>
#include "memorysaver.h"

const int CS = D5;
const int CAM_POWER_ON = D6;
ArduCAM myCAM(OV2640, CS);

void Capture() {
  Serial.println(F("SendCapture"));

  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

  uint32_t DataSize  = myCAM.read_fifo_length();
  if (DataSize >= MAX_FIFO_SIZE) //0x5FFFF      //384KByte
  {
    Serial.println(F("Over size."));
    DataSize = MAX_FIFO_SIZE;
  }
  if (DataSize == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  Serial.print(F("Capture Size :")); Serial.println(DataSize);
  
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  const char* host = "www.googleapis.com";
  const int httpsPort = 443;

  String start_request = api.getStartRequest_Jpeg("Capture", "From ArduCam");
  String end_request = api.getEndRequest();
  uint16_t full_length;
  full_length = start_request.length() + DataSize + end_request.length();
  String postHeader = api.getPostHeader(full_length);

  String result = "";

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("Connecting to: "); Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return ;
  }

  Serial.println("[Header]"); Serial.println(postHeader);
  Serial.println("[start_request]"); Serial.println(start_request);
  Serial.println("[end_request]"); Serial.println(end_request);
  client.print(postHeader + start_request);

  uint32_t len = DataSize;
  static const size_t bufferSize = 1024; // original value 4096 caused split pictures
  static uint8_t buffer[bufferSize] = {0xFF};
  while (len) {
    size_t will_copy = (len < bufferSize) ? len : bufferSize;
    SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
    if (!client.connected()) break;
    client.write(&buffer[0], will_copy);
    len -= will_copy;
  }

  client.println(end_request);
  myCAM.CS_HIGH();
  
  Serial.println("Receiving response");
  if (client.connected()) {
    if (client.find("HTTP/1.1 ")) {
      String status_code = client.readStringUntil('\r');
      Serial.print("Status code: "); Serial.println(status_code);
      if (status_code != "200 OK") {
        Serial.println("There was an error");
      }
    }

    if (client.find("\r\n\r\n")) {
      Serial.println(F("[Read Data]"));
    } else {
      Serial.println(F("[WARNING] Response Data is Nothing"));
    }

    String line = client.readStringUntil('\r');
    Serial.println(line);
    result += line;
  }


  Serial.println(F("Picture sent"));
}



void setup() {
  Nefry.setProgramName("ArduCAM OV2640 Sample");

  Nefry.enableSW();

  api.InitAPI();

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
  delay(1000);

}
void loop() {
  if (Nefry.readSW()) {
    Capture();
  }
}
