#include "mqttESP32.h"
mqttESP32::mqttESP32(
    const char *_host,
    const char *_topicPub, const char *_topicSub,
    const char *_clientId, const char *_token, const char *_caCert,
    void (*callback)(char *, byte *, unsigned int))
    : mqttConnectErr(0), mqttState(-1), mqttClient(espClient)
{
    host = const_cast<const char *>(_host);
    topicPub = const_cast<const char *>(_topicPub);
    topicSub = const_cast<const char *>(_topicSub);
    clientId = const_cast<const char *>(_clientId);
    token = const_cast<const char *>(_token);
    caCert = const_cast<const char *>(_caCert);

    mqttClient.setServer(host, 8883);
    mqttClient.setCallback(callback);
    espClient.setCACert(caCert);
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