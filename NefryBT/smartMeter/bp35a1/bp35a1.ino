#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include "intervalMs.h"

void setting()
{
    Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

HardwareSerial uart(1);

#define WAIT_CHECK_RESPONCE_MS 2000
#define RESPONCE_OK "OK"
#define SKTERM_ERR "ER10"
#define SKSCAN_FIND "EPANDESC"
#define SKSCAN_CHANNEL "Channel:"
#define SKSCAN_PANID "Pan ID:"
#define SKSCAN_ADDR "Addr:"
#define SKLL64_IPV6 "FE80"
#define SKJOIN_SUC "EVENT 25"
#define RESPONCE "ERXUDP"

bool isConnect;
String serviceID;
String servicePW;

String channel;
String panID;
String addr;
String ipv6;

// ループ周期(ms)

// 即時電力値の取得
#define LOOPTIME_GET_EP_VALUE 60 * 1000

void getEPValue()
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
        delay(100);
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
    channel = "";
    panID = "";
    addr = "";
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
        if (tmpRes.indexOf(SKTERM_ERR) < 0)
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
    tmpRes = chkResponse(SKSCAN_FIND, 30000);
    //Serial.println(tmpRes);
    if (tmpRes.indexOf(SKSCAN_FIND) < 0)
    {
        Serial.println(F("[Not Found SmartMeterDevice...]"));
        return;
    }
    int idx = tmpRes.indexOf(SKSCAN_CHANNEL) + 8;
    channel = tmpRes.substring(idx, idx + 2);
    idx = tmpRes.indexOf(SKSCAN_PANID) + 7;
    panID = tmpRes.substring(idx, idx + 4);
    idx = tmpRes.indexOf(SKSCAN_ADDR) + 5;
    addr = tmpRes.substring(idx, idx + 16);

    // スマートメーター機器の登録
    send("SKSREG S2 " + channel);
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Setting Channnel...]"));
        return;
    }
    send("SKSREG S3 " + panID);
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to Setting Pan ID...]"));
        return;
    }

    // IPv6値を取得する
    send("SKLL64 " + addr);
    ipv6 = chkResponse(SKLL64_IPV6, WAIT_CHECK_RESPONCE_MS);
    ipv6.trim();

    // PANA接続 ※末尾に改行が必要
    send("SKJOIN " + ipv6 + "\n");
    tmpRes = chkResponse(SKJOIN_SUC, 30000);
    if (tmpRes.indexOf(SKJOIN_SUC) < 0)
    {
        Serial.println(F("[Fail to Connect...]"));
        return;
    }

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

    interval<LOOPTIME_GET_EP_VALUE>::run([] {
        if (isConnect)
        {
            getEPValue();
            String tmp = chkResponse(RESPONCE, 15000);
            String resData = tmp.substring(tmp.lastIndexOf(" ") + 1);
            Serial.println(resData);
        }
    });
}