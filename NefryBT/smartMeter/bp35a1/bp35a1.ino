#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

HardwareSerial uart(1);

#define WAIT_CHECK_RESPONCE_MS 2000
#define RESPONCE_OK "OK"

bool isConnect;
String serviceID;
String servicePW;

String Channel;
String PanID;
String Addr;
String ipv6;

void send_ECHONETLiteComm()
{
    //コマンドバイト列
    int ECHONETLiteComm[16] = {0x10, 0x81, 0x00, 0x01, 0x05, 0xFF, 0x01, 0x02, 0x88, 0x01, 0x62, 0x02, 0xE7, 0x00, 0xE8, 0x00};
    //UDPハンドラ、宛先、宛先ポート番号(0x0E1A)、暗号化フラグ、送信データ長の送信(16byte) 、スペース(0x20)
    uart.print("SKSENDTO 1 " + ipv6 + " 0E1A 1 0010 ");
    int d = 0;
    for (d = 0; d <= 15; d++)
    {
        uart.write((int)ECHONETLiteComm[d]);
    }
    uart.println();
}

void send(String val)
{
    Serial.println("\n>>> " + val);
    uart.println(val);
}

// レスポンスの確認(キーワードを含むデータを取得)
String chkResponse(String keyword, unsigned long chkTime)
{
    String res = "";
    String readData = "";
    unsigned long waitTime = millis() + chkTime;

    while (waitTime >= millis())
    {
        readData = uart.readString();
        Serial.println("<<< " + readData);
        res += readData;
        if (res.indexOf(keyword) >= 0)
        {
            Serial.println(F("[Find keyword]"));
            break;
        }
    }
    return res;
}

// レスポンスの確認(OKが返ってくるかチェックするだけの場合)
bool chkResponceOK(unsigned long chkTime)
{
    if (chkResponse(RESPONCE_OK, chkTime).indexOf(RESPONCE_OK) >= 0)
    {
        Serial.println(F("[Responce Check OK !!!]"));
        return true;
    }
    return false;
}

void connect()
{
    Serial.println(F("[Start Connection...]"));
    Channel = "";
    PanID = "";
    Addr = "";
    ipv6 = "";

    String tmpRes = "";

    // バージョンの確認
    send("SKVER");
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Check Version...]"));
        return;
    }

    // PANAセッションの終了　接続ならOK,未接続ならER10が返ってくる
    send("SKTERM");
    tmpRes = chkResponse(RESPONCE_OK, WAIT_CHECK_RESPONCE_MS);
    if (tmpRes.indexOf(RESPONCE_OK) < 0)
    {
        if (tmpRes.indexOf("ER10") < 0)
        {
            Serial.println(F("[Fail to DisConnect...]"));
            return;
        }
    }

    // コマンドのエコーバックを無効にする
    send("SKSREG SFE 0");
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to DisEnable EchoBack...]"));
        return;
    }

    // 応答を16進ASCII文字で返すように
    send("WOPT 01");
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Setting Responce Pattern...]"));
        return;
    }

    // BルートサービスのIDを設定する
    send("SKSETRBID " + serviceID);
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Service ID...]"));
        return;
    }

    // Bルートサービスのパスワードを設定する
    send("SKSETPWD C " + servicePW);
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Service ID...]"));
        return;
    }

    // スマートメーターの存在をスキャンする
    send("SKSCAN 2 FFFFFFFF 6");
    tmpRes = chkResponse("EVENT 20", 15000);
    Serial.println(tmpRes);

    Serial.println(F("[Connected!!!]"));
    isConnect = true;
}

void setup()
{
    // (speed, type, RX:D2=23, TX:D3=19);
    uart.begin(115200, SERIAL_8N1, 23, 19);
    isConnect = false;
    serviceID = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    servicePW = "XXXXXXXXXXXX";
}

void loop()
{
    if (!isConnect)
    {
        connect();
    }
}