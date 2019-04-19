ESP-SensorCam用サンプルプログラム

【概要】
P2に接続されたArduCAM-Mini-2MPカメラモジュールで撮影したデータをSDカードにJPEG形式で保存するサンプルスケッチです。

【テスト環境】
Arduino IDE 1.8.5
ボードマネージャー ESP8266 Core Version2.4.1
ArduCAM-Mini-2MP
ArduCAMライブラリ

【ファイル構成】
Camera-Sample.ino : スケッチファイル

【使い方】
あらかじめ以下ArduCAMより提供されているライブラリをお使いのArduinoIDE用にインストールしておく必要があります。
https://github.com/ArduCAM/Arduino

以下サポートページを参考にArduCAM-Mini-2MP(OV2640センサー)に合わせてmemorysaver.hの設定を行なってください。
http://indoor.lolipop.jp/IndoorCorgiElec/ESP-SensorCam.php

カメラモジュールをP2コネクタに接続し、FAT32形式でフォーマット済みのmicroSDを基板上スロットに入れて、
電源を入れた後、スケッチを書きこんでください。
ピンの信号が一致する向きで取り付けてください。 逆向きに挿すと部品を痛めるおそれがあります。

プログラムがスタートするとシリアルモニタ上に進捗が表示され、SDカードにPHOTO.JPGという名前のファイルが作成されます。
SDカードをPCなどで開いて画像を確認してください。
