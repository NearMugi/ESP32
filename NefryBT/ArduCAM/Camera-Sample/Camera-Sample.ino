//ESP-WROOM-02 なので注意。
//http://indoor.lolipop.jp/IndoorCorgiElec/ESP-SensorCam.php




#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduCAM.h>
#include <SD.h>


const uint8_t pinLED = 14;
const uint8_t pinSW = 0;
const uint8_t pinArduCamCS = 15;
const uint8_t pinSDCS = 2;
const uint8_t pinSDA = 4;
const uint8_t pinSCL = 5;
const uint8_t pinMotion = 16;

ArduCAM arduCam(OV2640, pinArduCamCS);


void setup() {
  Serial.begin(115200);
  delay(1);
  Serial.print("\n\n-----------------------\n");
  Serial.print("Program Start\n");
  
  pinMode(pinLED, OUTPUT);   // LED
  pinMode(pinSW, INPUT);     // スイッチ、照度センサーINT信号
  pinMode(pinMotion, INPUT_PULLDOWN_16);  // モーションセンサー入力

  // SPI CSはHIGHにしておく
  pinMode(pinArduCamCS, OUTPUT);
  pinMode(pinSDCS, OUTPUT);
  digitalWrite(pinArduCamCS, HIGH);
  digitalWrite(pinSDCS, HIGH);
  

  // カメラ接続確認
  Serial.println("Verify camera module");
  Wire.begin();
  SPI.begin();

  // I2Cセンサーの確認
  uint8_t pid = 0;
  uint8_t vid = 0;
  arduCam.wrSensorReg8_8(0xFF, 0x01);
  arduCam.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  arduCam.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if((vid != 0x26 ) && (( !pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println("Camera Not Found (I2C)");
    return;
  }

  // SPIバッファメモリの確認
  arduCam.write_reg(ARDUCHIP_TEST1, 0xA5);
  uint8_t data = arduCam.read_reg(ARDUCHIP_TEST1);
  if(data != 0xA5){
    Serial.println("Camera Not Found (SPI)");
    return;
  }

  // 初期設定
  Serial.println("Config camera module");
  arduCam.set_format(JPEG);
  arduCam.InitCAM();
  arduCam.OV2640_set_JPEG_size(OV2640_1280x1024);  // 画像サイズ設定

  // JPEG画質設定
  arduCam.wrSensorReg8_8(0xFF, 0x00);
  arduCam.wrSensorReg8_8(0x44, 0x4);  // 0x4:画質優先 0x8:バランス 0xC:圧縮率優先

  delay(4000);  // カメラが安定するまで待つ
  
  arduCam.flush_fifo();
  arduCam.clear_fifo_flag();
  Serial.println("Start capture");
  arduCam.start_capture();    // 撮影情報をカメラモジュールのバッファーにメモリに転送
  while(!arduCam.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)){ // 終わるまで待つ
    delay(1);
  }

  // カメラモジュールのバッファーメモリからSDカードに転送
  Serial.println("Write to SD card");
  String path = "/photo.jpg";  // 保存するファイル名
  const uint8_t bufferSize = 32;  // ESP8266に確保するバッファーサイズ
  uint8_t*buf;
  buf = new uint8_t[bufferSize];
  uint8_t prev = 0;
  uint16_t index = 0;
  
  if(SD.begin(pinSDCS)){
    File file = SD.open(path, O_RDWR | O_CREAT);
    if(file){
      arduCam.CS_LOW();
      arduCam.set_fifo_burst();
      buf[index] = SPI.transfer(0x0);  // 先頭に不要な0x00が入るのを回避
      
      while(1){
        buf[index] = SPI.transfer(0x0);

        // JPEGファイルの最後を検出したら終了
        if(prev==0xFF && buf[index]==0xD9){
          arduCam.CS_HIGH();
          file.write(buf, index+1);
          break;
        }

        if(index==bufferSize-1){
          arduCam.CS_HIGH();
          file.write(buf, bufferSize);
          arduCam.CS_LOW();
          arduCam.set_fifo_burst();
          index = 0;
        }else{
          prev = buf[index];
          index++;
        }
        delay(0);
      }
      file.close();
      arduCam.CS_HIGH();
    }else{
      Serial.println("Could not open file on SD card");
    }
  }else{
    Serial.println("SD card error");
  }
  delete[] buf;

  Serial.println("Finished");
  
  
}


void loop() {

}
