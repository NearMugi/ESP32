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
                           windDirection(""),
                           wind(0.0),
                           sun(0.0),
                           snow(0.0),
                           humidity(0.0),
                           pressure(0.0){};
        unitAmedasData(float _t, float _r, String _wd, float _w, float _su, float _sn, float _h, float _p) : temperature(_t),
                                                                                                             rain(_r),
                                                                                                             windDirection(_wd),
                                                                                                             wind(_w),
                                                                                                             sun(_su),
                                                                                                             snow(_sn),
                                                                                                             humidity(_h),
                                                                                                             pressure(_p){};
        virtual ~unitAmedasData(){};
        float getTemperature() const { return temperature; };
        float getRain() const { return rain; };
        String getWindDirection() const { return windDirection; };
        float getWind() const { return wind; };
        float getSun() const { return sun; };
        float getSnow() const { return snow; };
        float getHumidity() const { return humidity; };
        float getPressure() const { return pressure; };

    private:
        float temperature;
        float rain;
        String windDirection;
        float wind;
        float sun;
        float snow;
        float humidity;
        float pressure;
    };

public:
    amedas() : _tblData(), _lastData(), posID("00000"){};
    amedas(String _id) : _tblData(), _lastData(), posID(_id){};
    amedas(int _id) : _tblData(), _lastData(), posID(String(_id)){};
    ~amedas(){};
    bool getData();
    float getLastTemperature() const { return _lastData.getTemperature(); };
    float getLastPressure() const { return _lastData.getPressure(); };

private:
    std::vector<unitAmedasData> _tblData;
    unitAmedasData _lastData;
    String posID;
};
#endif