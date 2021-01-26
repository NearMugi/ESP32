#ifndef MQTT_ESP32_H
#define MQTT_ESP32_H
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class mqttESP32
{
public:
    mqttESP32(
        const char *_host,
        const char *_topicPub, const char *_topicSub,
        const char *_clientId, const char *_token, const char *_caCert,
        void (*callback)(char *, byte *, unsigned int));
    virtual ~mqttESP32(){};
    int chkConnect();
    void loop();
    void publish(char *bufferData);
    int getMqttState() const { return mqttState; };
    int getMqttConnectErr() const { return mqttConnectErr; };

private:
    // デフォルトコンストラクタ、コピーコンストラクタ、コピー代入演算子を禁止
    mqttESP32();
    mqttESP32(const mqttESP32 &);
    mqttESP32 &operator=(const mqttESP32 &);
    WiFiClientSecure espClient;
    PubSubClient mqttClient;

    const char *host;
    const char *topicPub;
    const char *topicSub;
    const char *clientId;
    const char *token;
    const char *caCert;
    int mqttConnectErr;
    int mqttState;

    void reConnect();
};

#endif