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

// 即時電力値・電流値・累積電力値を取得
#define LOOPTIME_GET_EP_VALUE 60 * 1000
// 接続確認
#define LOOPTIME_CHECK_CONNECT 30 * 1000

// 即時電力値・電流値・累積電力値の取得
void getValue()
{
    Serial.println("[Start getElectricPowerValue]");
    int cmdSize = 12 + 2 * 5;
    String cmdSizeString = String(cmdSize, HEX);
    cmdSizeString.toUpperCase();
    int cmdSizeStringSize = cmdSizeString.length();
    for (int i = 0; i < 4 - cmdSizeStringSize; i++)
    {
        cmdSizeString = "0" + cmdSizeString;
    }
    //コマンドバイト列
    // EHD, TID, SEOJ, DEOJ, ESV, OPC, EPCn+PDCn
    int ECHONETLiteComm[cmdSize] = {
        0x10, 0x81,
        0x00, 0x01,
        0x05, 0xFF, 0x01,
        0x02, 0x88, 0x01,
        0x62,
        0x05,
        0xE7, 0x00,
        0xE8, 0x00,
        0xD3, 0x00,
        0xE1, 0x00,
        0xEA, 0x00};

    //UDPハンドラ、宛先、宛先ポート番号(0x0E1A)、暗号化フラグ、送信データ長の送信(16byte) 、スペース(0x20)
    uart.print("SKSENDTO 1 " + ipv6 + " 0E1A 1 " + cmdSizeString + " ");
    for (int d = 0; d < cmdSize; d++)
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

unsigned int hexToDec(String hexString)
{

    unsigned int decValue = 0;
    int nextInt;

    for (int i = 0; i < hexString.length(); i++)
    {

        nextInt = int(hexString.charAt(i));
        if (nextInt >= 48 && nextInt <= 57)
            nextInt = map(nextInt, 48, 57, 0, 9);
        if (nextInt >= 65 && nextInt <= 70)
            nextInt = map(nextInt, 65, 70, 10, 15);
        if (nextInt >= 97 && nextInt <= 102)
            nextInt = map(nextInt, 97, 102, 10, 15);
        nextInt = constrain(nextInt, 0, 15);

        decValue = (decValue * 16) + nextInt;
    }

    return decValue;
}

float convertPowerUnit(uint32_t int_power_unit)
{
    switch (int_power_unit)
    {
    case 0x0:
        return 1;
    case 0x1:
        return 0.1;
    case 0x2:
        return 0.01;
    case 0x3:
        return 0.001;
    case 0x4:
        return 0.0001;
    case 0xA:
        return 10;
    case 0xB:
        return 100;
    case 0xC:
        return 1000;
    case 0xD:
        return 10000;
    default:
        Serial.println("convert error" + String(int_power_unit, DEC));
        return 0;
    }
};

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
            getValue();
            String tmp = chkResponse(10000);
            if (tmp.indexOf(RESPONCE_EP_VALUE) > 0)
            {
                String resData = tmp.substring(tmp.lastIndexOf(" ") + 1);
                Serial.print(F("[Get Electric Power] : "));
                Serial.println(resData);
                tmp = resData.substring(24);
                String stringE7 = tmp.substring(4, 12);
                String stringE8 = tmp.substring(12 + 4, 12 + 12);
                String stringD3 = tmp.substring(24 + 4, 24 + 12);
                String stringE1 = tmp.substring(36 + 4, 36 + 6);
                String stringEA = tmp.substring(42 + 4);

                Serial.println(stringE7);
                Serial.println(stringE8);
                Serial.println(stringD3);
                Serial.println(stringE1);
                Serial.println(stringEA);

                // 瞬時電力(0.1A)・電流値(W)
                float epA = (float)(hexToDec(stringE7)) * 0.1;
                float epkW = (float)hexToDec(stringE8) * 0.001;
                // 係数
                float coefficient = hexToDec(stringD3);
                // 積算電力量単位
                float powerInit = convertPowerUnit(hexToDec(stringE1));

                // 累積電力値
                int year = hexToDec(stringEA.substring(0, 4));
                int month = hexToDec(stringEA.substring(4, 4 + 2));
                int day = hexToDec(stringEA.substring(6, 6 + 2));
                int hour = hexToDec(stringEA.substring(8, 8 + 2));
                int minute = hexToDec(stringEA.substring(10, 10 + 2));
                int second = hexToDec(stringEA.substring(12, 12 + 2));
                char date[15] = "";
                sprintf(date, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);

                float totalkWh = hexToDec(stringEA.substring(14));
                totalkWh *= coefficient * powerInit;

                Serial.println(epkW);
                Serial.println(epA);
                Serial.println(powerInit);
                Serial.println(coefficient);
                Serial.println(date);
                Serial.println(totalkWh);
            }
        }
    });
}