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

  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();//撮影情報をカメラモジュールのバッファーにメモリに転送
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)) { // 終わるまで待つ
    delay(1);
  }

  //カメラモジュールのバッファーメモリからデータを取得
  static uint32_t MAX_BUF_SIZE = 70000;
  static uint8_t tmpbuf[70000];

  uint32_t len  = myCAM.read_fifo_length();
  Serial.print(F("Buffer Size :")); Serial.println(len);
  if (len >= MAX_BUF_SIZE)
  {
    Serial.println(F("Over size."));
    len = MAX_BUF_SIZE;
  }
  if (len == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }

  uint32_t index = 0;
  uint8_t prev = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  bool isHeader = false;
  while (len--) {
    tmpbuf[index] = SPI.transfer(0x00);
    //Serial.print(tmpbuf[index]); Serial.print(F(", "));

    //ヘッダーを探す(0xFF,0xD8)
    if (!isHeader) {
      if (prev == 0xFF && tmpbuf[index] == 0xD8) {
        Serial.println(F("JPEG First Data is Found"));
        tmpbuf[0] = 0xFF;
        tmpbuf[1] = 0xD8;
        index = 2;
        isHeader = true;
      } else {
        prev = tmpbuf[index];
      }
      continue;
    }

    // JPEGファイルの最後を検出したら終了(0xFF,0xD9)
    if (prev == 0xFF && tmpbuf[index] == 0xD9) {
      Serial.println(F("JPEG Last Data is Found"));
      myCAM.CS_HIGH();
      break;
    }

    prev = tmpbuf[index];
    index++;

  }
  myCAM.CS_HIGH();
  uint32_t DataSize = index;
  Serial.print(F("JPEG Data Size: ")); Serial.println(DataSize);


  //GoogleDriveへポスト
  const char* host = "www.googleapis.com";
  const int httpsPort = 443;

  String start_request = api.getStartRequest_Jpeg("Capture", "From ArduCam");
  String end_request = api.getEndRequest();
  uint32_t full_length;
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

  //JPEGデータ
  static const uint16_t bufferSize = 2048; // original value 4096 caused split pictures
  uint8_t cnt = 0;
  while (DataSize) {
    size_t will_copy = (DataSize < bufferSize) ? DataSize : bufferSize;
    if (!client.connected()) break;
    client.write(&tmpbuf[0 + bufferSize * cnt], will_copy);
    DataSize -= will_copy;
    cnt++;
  }

  client.println(end_request);


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

  //set the CS as an output:
  pinMode(CS, OUTPUT);
  pinMode(CAM_POWER_ON , OUTPUT);
  digitalWrite(CS, HIGH);
  digitalWrite(CAM_POWER_ON, HIGH);

  //カメラ接続確認
  Serial.println(F("Verify camera module"));

  SPI.begin();
  SPI.setFrequency(4000000); //4MHz
  Wire.begin();

  //I2Cセンサーの確認
  uint8_t pid = 0;
  uint8_t vid = 0;
  myCAM.wrSensorReg8_8(0xFF, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( !pid != 0x41 ) || ( pid != 0x42 ))) {
    Serial.println("Camera Not Found (I2C)");
    return;
  }
  Serial.println(F("Camera Found (I2C)"));

  //SPIバッファメモリの確認
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("Camera Not Found (SPI)"));
    return;
  }
  Serial.println(F("Camera Found (SPI)"));

  //初期設定
  Serial.println(F("Config camera module"));
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  //OV2640_320x240, OV2640_640x480, OV2640_800x600, OV2640_1280x1024, OV2640_1600x1200
  myCAM.OV2640_set_JPEG_size(OV2640_800x600); 

  //JPEG画質設定
  myCAM.wrSensorReg8_8(0xFF, 0x00);
  myCAM.wrSensorReg8_8(0x44, 0x4);  // 0x4:画質優先 0x8:バランス 0xC:圧縮率優先

  delay(4000);  // カメラが安定するまで待つ
  Serial.println(F("Camera setting is completed !"));

  Capture();

}
void loop() {
  if (Nefry.readSW()) {
    Capture();
  }
}
