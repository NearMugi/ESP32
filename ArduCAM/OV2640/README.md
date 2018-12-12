# OV2640を動かす
http://www.arducam.com/arducam-mini-released/

The OV2640 I2C slave address is 0x60 for write and 0x61 for read.

https://github.com/ArduCAM/ArduCAM_ESP32S_UNO


## ライブラリをコピーする
C:\Users\(ユーザー)\AppData\Local\Arduino15\packages\ArduCAM_ESP32S_UNO\hardware\esp32\2.0.0\libraries\
ArduCAM
ESP32WebServer

(ドキュメント)\Arduino\libraries

## ライブラリのmemorysaver.hを編集する  
以下の定義を有効にする  
#define OV2640_MINI_2MP  
#define OV2640_CAM  

## サンプルプログラム(ArduCAM_ESP32_Capture)の解析

### ピン配置をどうする？  
VSPIとSDA,SCLを使えば良いと思う。  
<table>
    <tr>
        <td>機能</td>
        <td>サンプルプログラム</td>
        <td>NefryBT</td>
    </tr>
    <tr>
        <td>CS</td>
        <td>17?</td>
        <td>D5</td>
    </tr>
    <tr>
        <td>MOSI</td>
        <td>?</td>
        <td>D2</td>
    </tr>
    <tr>
        <td>MISO</td>
        <td>?</td>
        <td>D3</td>
    </tr>
    <tr>
        <td>SCLK</td>
        <td>?</td>
        <td>D4</td>
    </tr>
    <tr>
        <td>SDA</td>
        <td>?</td>
        <td>D1</td>
    </tr>
    <tr>
        <td>SCL</td>
        <td>?</td>
        <td>D0</td>
    </tr>
    <tr>
        <td>GND</td>
        <td>GND</td>
        <td>GND</td>
    </tr>
    <tr>
        <td>5v</td>
        <td>D10(5)?</td>
        <td>5v</td>
    </tr>
  
</table>

#### CAM_POWER_ON(=D10)はESP32のIO番号5(VSPI CS)  
http://trac.switch-science.com/wiki/espr_one32  
![espr one 32 esp-wroom-32 arduino](https://user-images.githubusercontent.com/25577827/49706042-869b7980-fc66-11e8-8321-84c2bbef210c.PNG)  
#### ESP32のピン配置  
![esp32_devkitc_pinout_01](https://user-images.githubusercontent.com/25577827/49706167-79cb5580-fc67-11e8-9c79-ac03d4d5cee4.png)  
![esp32_pinmap](https://user-images.githubusercontent.com/25577827/49712200-a6db3080-fc86-11e8-8c2d-d78313e0753f.png)
#### NefryBT R2　ピン配置  
![153ddd38df8d84c15b1f0cefb13eb5e4-768x425](https://user-images.githubusercontent.com/25577827/49710207-6cb86180-fc7b-11e8-8e6a-e0398bdf4f1a.png)  
<img width="384" alt="a8e4eea9c7118306a960a601ebcc4778-768x660" src="https://user-images.githubusercontent.com/25577827/49710209-6e822500-fc7b-11e8-84a7-914ad6560271.png">  

## データの流れ  
NefryBTからjpegデータをどこかにPOSTする。  
ラズパイ3？
