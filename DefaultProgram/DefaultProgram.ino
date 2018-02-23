#include <NefryIFTTT.h>
#include <WiFiClientSecure.h>

String Event, SecretKey, LineAuth, SendMessageLINE;
WiFiClientSecure client;
String StrPerEncord(const char* c_str);
String escapeParameter(String param);
void LineSend();

void setup() {
  Nefry.setStoreTitle("SecretKey", 0); //Nefry DataStoreのタイトルを指定
  Nefry.setStoreTitle("Event", 1);    //Nefry DataStoreのタイトルを指定
  Nefry.setStoreTitle("LINE Auth", 2); //Nefry DataStoreのタイトルを指定
  Nefry.setStoreTitle("LINEMessage", 3);    //Nefry DataStoreのタイトルを指定
  SecretKey = Nefry.getStoreStr(0);   //Nefry DataStoreからデータを取得
  Event = Nefry.getStoreStr(1);       //Nefry DataStoreからデータを取得
  LineAuth = Nefry.getStoreStr(2);   //Nefry DataStoreからデータを取得
  SendMessageLINE = Nefry.getStoreStr(3);       //Nefry DataStoreからデータを取得
  Nefry.enableSW();                   //SW有効化
  Nefry.setProgramName("NefryBT Default Program");
}

void loop() {
  if (Nefry.readSW()) {               //SWを押した時
    if (!SecretKey.equals("") && !Event.equals("")) {
      if (!IFTTT.send(Event, SecretKey)) {//IFTTTにデータを送信
        Nefry.setLed(255, 0, 0);        //Errの時、赤色点灯
      }
    }
    if (!LineAuth.equals("")) {
      if (SendMessageLINE.equals(""))SendMessageLINE = "Welcome to the NefryBT world!";
      LineSend();                     //LINE送信
    }
    Nefry.ndelay(1000);               //送信後1秒間待つ
  }
  Nefry.setLed(random(255), random(255), random(255));
  Nefry.ndelay(500);               //送信後0.5秒間待つ
}

void LineSend() {
  const char* host = "notify-api.line.me";
  Serial.println("\nStarting connection to server...");
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
    String url = "/api/notify";
    url += "?message=";
    url += StrPerEncord(escapeParameter(SendMessageLINE).c_str());
    Serial.println(StrPerEncord(SendMessageLINE.c_str()));
    Serial.println(url);
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Authorization: Bearer " + LineAuth + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    client.println();
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    Serial.println("closing connection");
  }
}
//********************UTF-8文字列をパーセントエンコード*************************
String StrPerEncord(const char* c_str) {
  uint16_t i = 0;
  String str_ret = "";
  char c1[3], c2[3], c3[3];

  while (c_str[i] != '\0') {
    if (c_str[i] >= 0xC2 && c_str[i] <= 0xD1) { //2バイト文字
      sprintf(c1, "%2x", c_str[i]);
      sprintf(c2, "%2x", c_str[i + 1]);
      str_ret += "%" + String(c1) + "%" + String(c2);
      i = i + 2;
    } else if (c_str[i] >= 0xE2 && c_str[i] <= 0xEF) {
      sprintf(c1, "%2x", c_str[i]);
      sprintf(c2, "%2x", c_str[i + 1]);
      sprintf(c3, "%2x", c_str[i + 2]);
      str_ret += "%" + String(c1) + "%" + String(c2) + "%" + String(c3);
      i = i + 3;
    } else {
      str_ret += String(c_str[i]);
      i++;
    }
  }
  return str_ret;
}

String escapeParameter(String param) {
  param.replace("%", "%25");
  param.replace("+", "%2B");
  param.replace(" ", "+");
  param.replace("\"", "%22");
  param.replace("#", "%23");
  param.replace("$", "%24");
  param.replace("&", "%26");
  param.replace("'", "%27");
  param.replace("(", "%28");
  param.replace(")", "%29");
  param.replace("*", "%2A");
  param.replace(",", "%2C");
  param.replace("/", "%2F");
  param.replace(":", "%3A");
  param.replace(";", "%3B");
  param.replace("<", "%3C");
  param.replace("=", "%3D");
  param.replace(">", "%3E");
  param.replace("?", "%3F");
  param.replace("@", "%40");
  param.replace("[", "%5B");
  param.replace("\\", "%5C");
  param.replace("]", "%5D");
  param.replace("^", "%5E");
  param.replace("'", "%60");
  param.replace("{", "%7B");
  param.replace("|", "%7C");
  param.replace("}", "%7D");
  return param;
}
