#include "mqttESP32.h"
#include "mqttConfig.h"
mqttESP32::mqttESP32(void (*callback)(char *, byte *, unsigned int)) : mqttConnectErr(0), mqttState(-1), mqttClient(host, 8883, espClient)
{
    espClient.setCACert(beebottle_ca_cert);
    mqttClient.setCallback(callback);
};

int mqttESP32::chkConnect()
{
    mqttState = mqttClient.state();
    if (mqttState != 0)
        reConnect();

    if (mqttState != 0)
    {
        mqttConnectErr++;
    }
    else
    {
        mqttConnectErr = 0;
    }
    return mqttState;
}

void mqttESP32::reConnect()
{
    if (mqttClient.connect(clientId, token, NULL))
    {
        mqttClient.subscribe(topicSub);
    }
    mqttState = mqttClient.state();
}

void mqttESP32::loop() { mqttClient.loop(); };
void mqttESP32::publish(char *bufferData) { mqttClient.publish(topicPub, bufferData); };