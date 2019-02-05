#include <Nefry.h>
#include <NefryDisplay.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

#include "googleAPI.h"
googleAPI api;
// Data Store
// 4:Storage Bucket
// 5:Refresh Token
// 6:Client ID
// 7:Client Secret
// 8:Parent Folder

//true : DriveにPOST false : StorageにPOST
#define POST_DRIVE true

//date
#include <time.h>
#define JST     3600*9

#include "ArduCAM.h"
#include <SPI.h>
#include "memorysaver.h"

const int CS = D5;
ArduCAM myCAM(OV2640, CS);

bool ArduCAM_Init() {
  Serial.println(F("Start Camera Setting"));

  //set the CS as an output:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  //カメラ接続確認
  Serial.println(F("Verify camera module..."));

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
    return false;
  }
  Serial.println(F("Camera Found (I2C)"));

  //SPIバッファメモリの確認
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("Camera Not Found (SPI)"));
    return false;
  }
  Serial.println(F("Camera Found (SPI)"));

  //初期設定
  Serial.println(F("Config camera module..."));
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  //OV2640_320x240, OV2640_640x480, OV2640_800x600, OV2640_1280x1024, OV2640_1600x1200
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);

  //JPEG画質設定
  myCAM.wrSensorReg8_8(0xFF, 0x00);
  myCAM.wrSensorReg8_8(0x44, 0x4);  // 0x4:画質優先 0x8:バランス 0xC:圧縮率優先

  delay(4000);  // カメラが安定するまで待つ
  Serial.println(F("Camera setting is completed !"));

  return true;
}

void ArduCAM_Capture(String fn, String comment) {
  Serial.println(F("Start Caputure and Send"));

  String msg = "";

  NefryDisplay.clear();
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, F("File:"));
  NefryDisplay.drawString(20, 0, fn);
  NefryDisplay.drawString(0, 10, F("Comment:"));
  NefryDisplay.drawString(50, 10, comment);
  NefryDisplay.display();


  //撮影情報をカメラモジュールのバッファーにメモリに転送
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));

  uint32_t ReadSize  = myCAM.read_fifo_length();
  Serial.print(F("Read Size :")); Serial.println(ReadSize);
  if (ReadSize >= MAX_FIFO_SIZE) {
    Serial.println(F("Over size."));
    ReadSize = MAX_FIFO_SIZE;
  }
  if (ReadSize == 0 ) Serial.println(F("Size is 0."));


  //ポスト
  String start_request = api.getStartRequest_Jpeg(fn, comment);
  String end_request = api.getEndRequest();
  uint32_t full_length;
  full_length = start_request.length() + ReadSize + end_request.length();

  //Drive or GCP Storage
  String postHeader = "";
  if (POST_DRIVE) {
    postHeader = api.getPostHeader(api.postHeader_base_drive, full_length);
  } else {
    postHeader = api.getPostHeader(api.postHeader_base_storage, full_length);
  }

  String result = "";

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("Connecting to: "); Serial.println(api.host);

  if (!client.connect(api.host, api.httpsPort)) {
    msg = F("connection failed");
    SetDebugMsg(msg, 20);
    return;
  }

  Serial.println("[Header]"); Serial.println(postHeader);
  Serial.println("[start_request]"); Serial.println(start_request);
  Serial.println("[end_request]"); Serial.println(end_request);
  client.print(postHeader + start_request);

  //JPEGデータ
  static const size_t bufferSize = 2048;
  static uint8_t buffer[bufferSize] = {0xFF};
  uint32_t index = 0;
  uint32_t sizeCnt = 0;
  uint8_t now = 0;
  uint8_t prev = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  bool isHeader = false;
  while (ReadSize--) {
    prev = now;
    now = SPI.transfer(0x00);

    //ヘッダーを探す(0xFF,0xD8)
    if (!isHeader) {
      if (prev == 0xFF && now == 0xD8) {
        Serial.println(F("JPEG First Data is Found"));
        buffer[0] = 0xFF;
        buffer[1] = 0xD8;
        index = 2;
        sizeCnt = 2;
        isHeader = true;
      }
      continue;
    }

    //ヘッダーが見つかったあと
    sizeCnt++;

    // JPEGファイルの最後を検出したら終了(0xFF,0xD9)
    if (prev == 0xFF && now == 0xD9) {
      Serial.println(F("JPEG Last Data is Found"));
      buffer[index++] = now;
      client.write(&buffer[0], index);
      myCAM.CS_HIGH();
      break;
    }

    if (index < bufferSize) {
      buffer[index] = now;
      index++;

    } else {
      if (!client.connected()) break;
      client.write(&buffer[0], bufferSize);
      index = 0;
      buffer[index++] = now;
    }

  }
  myCAM.CS_HIGH();
  ReadSize += 1;

  msg = F("Finished Reading Buffer");
  SetDebugMsg(msg, 20);

  msg = F("JPEG Data Size: ");
  msg += sizeCnt;
  SetDebugMsg(msg, 30);

  msg = F("Remaining Data Size: ");
  msg += ReadSize;
  //SetDebugMsg(msg, 40);
  Serial.println(msg);

  client.println(end_request);
  //バッファーメモリサイズと画像サイズが異なるため、full_lengthに達していない。
  //足りない分の帳尻を合わせる
  uint8_t tmpbuf[ReadSize] = {0x00};
  client.write(&tmpbuf[0], ReadSize);

  Serial.println(F("Receiving response"));
  if (client.connected()) {
    if (client.find("HTTP/1.1 ")) {
      String status_code = client.readStringUntil('\r');
      Serial.print(F("Status code: ")); Serial.println(status_code);
      if (status_code != "200 OK") {
        msg = F("[ERR] Status code:");
        msg += status_code;
        SetDebugMsg(msg, 50);
      }
    }

    if (client.find("\r\n\r\n")) {
      result = client.readStringUntil('\r');
      msg = F("Success!!!");
      SetDebugMsg(msg, 50);
      Serial.println(result);

    } else {
      msg = F("[WARNING] Response Data is Nothing");
      SetDebugMsg(msg, 50);
    }

  }

  Serial.println(F("Picture Sending is End"));
}

void SetDebugMsg(String msg, int pos) {
  Serial.println(msg);
  NefryDisplay.drawString(0, pos, msg);
  NefryDisplay.display();
}

void setup() {
  Nefry.setProgramName("ArduCAM OV2640 Sample");

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  //date
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  Nefry.enableSW();

  NefryDisplay.clear();
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(0, 0, F("[Post Jpeg File]"));
  NefryDisplay.drawString(0, 10, F("Init API Setting..."));
  NefryDisplay.display();

  api.InitAPI();

  NefryDisplay.drawString(0, 20, F("Success in Setting!"));
  NefryDisplay.drawString(0, 30, F("Camera Setting..."));
  NefryDisplay.display();

  bool isSuc = ArduCAM_Init();
  if (isSuc) {
    NefryDisplay.drawString(0, 40, F("Success in Camera Setting"));
    NefryDisplay.drawString(0, 50, F("Waiting Push Btn..."));
  } else {
    NefryDisplay.drawString(0, 40, F("Fail in Camera Setting"));
  }

  NefryDisplay.display();
  Nefry.ndelay(10);


}
void loop() {
  if (Nefry.readSW()) {

    //日付を取得する
    time_t  t = time(NULL);
    struct tm *tm;
    tm = localtime(&t);
    char _fn[20] = "";
    sprintf(_fn, "%04d%02d%02d_%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    String fn = "";
    //StorageにPOSTするときはフォルダ名も含める
    if(!POST_DRIVE){
      if(api.parentFolder.length() > 0) fn += api.parentFolder + "/";
    }
    fn += String(_fn);
    
    String comment = "From ArduCam";

    ArduCAM_Capture(fn, comment);
  }
}
