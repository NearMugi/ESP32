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
#define RESPONCE_EP_VALUE "1081000102880105FF0172"

bool isConnect;
String serviceID;
String servicePW;

String channel;
String panID;
String addr;
String ipv6;

// ループ周期(ms)

// 即時電力値・電流値を取得
#define LOOPTIME_GET_EP_VALUE 60 * 1000
// 累積電力値を取得
#define LOOPTIME_GET_TOTAL_EP_VALUE 60 * 1 * 1000
// 接続確認
#define LOOPTIME_CHECK_CONNECT 30 * 1000

void getValue(int *property)
{
    int propertySize = sizeof(property);
    const int cmdSize = 12 + propertySize;
    String cmdSizeString = String(cmdSize, HEX);
    cmdSizeString.toUpperCase();
    int cmdSizeStringSize = cmdSizeString.length();
    for (int i = 0; i < 4 - cmdSizeStringSize; i++)
    {
        cmdSizeString = "0" + cmdSizeString;
    }
    Serial.println(cmdSize);
    Serial.println(cmdSizeString);

    //コマンドバイト列
    // EHD, TID, SEOJ, DEOJ, ESV, OPC, EPC1+PDC1, EPC2+PDC2
    int ECHONETLiteComm[cmdSize] = {0x00};
    // EHD
    ECHONETLiteComm[0] = 0x10;
    ECHONETLiteComm[1] = 0x81;
    // TID
    ECHONETLiteComm[2] = 0x00;
    ECHONETLiteComm[3] = 0x01;
    // SEOJ
    ECHONETLiteComm[4] = 0x05;
    ECHONETLiteComm[5] = 0xFF;
    ECHONETLiteComm[6] = 0x01;
    // DEOJ
    ECHONETLiteComm[7] = 0x02;
    ECHONETLiteComm[8] = 0x88;
    ECHONETLiteComm[9] = 0x01;
    // ESV
    ECHONETLiteComm[10] = 0x62;
    // OPC
    ECHONETLiteComm[11] = propertySize / 2;
    // EPC + PDC
    for (int i = 0; i < propertySize; i++)
    {
        ECHONETLiteComm[12 + i] = property[i];
        Serial.println(ECHONETLiteComm[12 + i]);
    }

    //UDPハンドラ、宛先、宛先ポート番号(0x0E1A)、暗号化フラグ、送信データ長の送信(16byte) 、スペース(0x20)
    uart.print("SKSENDTO 1 " + ipv6 + " 0E1A 1 " + cmdSizeString + " ");
    int d = 0;
    for (d = 0; d < cmdSize; d++)
    {
        uart.write((int)ECHONETLiteComm[d]);
    }
    uart.println();
}

// 即時電力値・電流値の取得
void getEPValue()
{
    Serial.println("[Start getEPValue]");
    int propertyData[4] = {0xE7, 0x00, 0xE8, 0x00};
    getValue(propertyData);
}

// 累積電力値の取得
void getTotalEPValue()
{
    Serial.println("[Start getTotalEPValue]");
    int propertyData[6] = {0xD3, 0x00, 0xE1, 0x00, 0xEA, 0x00};
    getValue(propertyData);
}

void send(String val)
{
    Serial.println("\n>>> " + val);
    uart.println(val);
}

// レスポンスの確認(キーワードを含むデータを取得)
String chkResponse(unsigned long chkTime)
{
    String res = "";
    String readData = "";
    unsigned long waitTime = millis() + chkTime;
    while (waitTime >= millis())
    {
        if (uart.available())
        {
            readData = uart.readString();
            Serial.println("<<< " + readData);
            res += readData;
        }
    }
    return res;
}

// レスポンスの確認(OKが返ってくるかチェックするだけの場合)
bool chkResponceOK(unsigned long chkTime)
{
    if (chkResponse(chkTime).indexOf(RESPONCE_OK) >= 0)
    {
        //Serial.println(F("[Responce Check OK !!!]"));
        return true;
    }
    return false;
}

void connect()
{
    Serial.println(F("\n[Start Connection...]"));
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
    tmpRes = chkResponse(WAIT_CHECK_RESPONCE_MS);
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
    tmpRes = chkResponse(20000);
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
    ipv6 = chkResponse(WAIT_CHECK_RESPONCE_MS);
    ipv6.trim();

    // PINGを投げる
    send("SKPING " + ipv6);
    if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
    {
        Serial.println(F("[Fail to throw ping...]"));
        return;
    }

    // PANA接続
    send("SKJOIN " + ipv6);
    tmpRes = chkResponse(20000);
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

    interval<LOOPTIME_CHECK_CONNECT>::run([] {
        if (isConnect)
        {
            // PINGを投げる
            send("SKPING " + ipv6);
            if (!chkResponceOK(WAIT_CHECK_RESPONCE_MS))
            {
                Serial.println(F("[Fail to throw ping...]"));
                isConnect = false;
            }
        }
    });

    interval<LOOPTIME_GET_EP_VALUE>::run([] {
        if (isConnect)
        {
            getEPValue();
            String tmp = chkResponse(10000);
            if (tmp.indexOf(RESPONCE_EP_VALUE) > 0)
            {
                String resData = tmp.substring(tmp.lastIndexOf(" ") + 1);
                Serial.print(F("[Get Electric Power] : "));
                Serial.println(resData);
            }
        }
    });

    interval<LOOPTIME_GET_TOTAL_EP_VALUE>::run([] {
        if (isConnect)
        {
            getTotalEPValue();
            String tmp = chkResponse(10000);
            if (tmp.indexOf(RESPONCE_EP_VALUE) > 0)
            {
                String resData = tmp.substring(tmp.lastIndexOf(" ") + 1);
                Serial.print(F("[Get Total Electric Power] : "));
                Serial.println(resData);
            }
        }
    });
}