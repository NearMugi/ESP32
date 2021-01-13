#ifndef AMEDAS_H
#define AMEDAS_H
#include <vector>
#include <HTTPClient.h>

class amedas
{
    class unitAmedasData
    {
    public:
        unitAmedasData() : temperature(0.0),
                           rain(0.0),
                           wind(0.0),
                           sun(0.0),
                           snow(0.0),
                           humidity(0.0),
                           pressure(0.0){};
        virtual ~unitAmedasData(){};

        bool set(float _t, float _r, float _w, float _su, float _sn, float _h, float _p);

    private:
        float temperature;
        float rain;
        float wind;
        float sun;
        float snow;
        float humidity;
        float pressure;
    };

public:
    amedas() : _tblData(), posID("44132"){};
    amedas(String _id) : _tblData(), posID(_id){};
    amedas(int _id) : _tblData(), posID(String(_id)){};
    ~amedas(){};
    bool getData();

private:
    std::vector<unitAmedasData> _tblData;
    String posID;
};
#endif