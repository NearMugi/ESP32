# OV2640を動かす
http://www.arducam.com/arducam-mini-released/

The OV2640 I2C slave address is 0x60 for write and 0x61 for read.

https://github.com/ArduCAM/ArduCAM_ESP32S_UNO

ライブラリをコピーする
C:\Users\(ユーザー)\AppData\Local\Arduino15\packages\ArduCAM_ESP32S_UNO\hardware\esp32\2.0.0\libraries\
ArduCAM
ESP32WebServer

(ドキュメント)\Arduino\libraries


◆サンプルプログラム(ArduCAM_ESP32_Capture)の解析

CAM_POWER_ON(=D10)はESP32のIO番号5(VSPI CS)  
http://trac.switch-science.com/wiki/espr_one32  
![espr one 32 esp-wroom-32 arduino](https://user-images.githubusercontent.com/25577827/49706042-869b7980-fc66-11e8-8321-84c2bbef210c.PNG)
