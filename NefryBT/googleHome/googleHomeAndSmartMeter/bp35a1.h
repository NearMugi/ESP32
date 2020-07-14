#ifndef bp35a1_H
#define bp35a1_H
#include <Nefry.h>

#define WAIT_CHECK_RESPONCE_MS 2000
#define RESPONCE_OK "OK"
#define SKTERM_ERR "ER10"
#define SKSCAN_FIND "EPANDESC"
#define SKSCAN_CHANNEL "Channel:"
#define SKSCAN_PANID "Pan ID:"
#define SKSCAN_ADDR "Addr:"
#define SKLL64_IPV6 "FE80"
#define SKJOIN_SUC "EVENT 25"
#define RESPONCE_EP_VALUE " 002E 1081000102880105FF017205"

class bp35a1
{
public:
    bp35a1();
    void init(int PIN_RX, int PIN_TX, String id, String pw);
    void connect();
    void chkConnect();
    void getEPValue();
    void reConnect();

    // 瞬時電力(A)
    float epA;
    // 電流値(kW)
    float epkW;
    // 累積電力値(kwh)
    float totalkWh;
    // 累積電力値の対象時間
    String date;

private:
    bool isPhysicalConnect;
    bool isConnect;
    String serviceID;
    String servicePW;

    String channel;
    String panID;
    String addr;
    String ipv6;

    unsigned int hexToDec(String hexString);
    float convertPowerUnit(uint32_t int_power_unit);
    void send(String val);
    void getValue();
    String chkResponse(unsigned long chkTime);
    bool chkResponceOK(unsigned long chkTime);
};

#endif