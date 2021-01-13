#include "amedas.h"

bool amedas::unitAmedasData::set(float _t, float _r, float _w, float _su, float _sn, float _h, float _p)
{
    temperature = _t;
    rain = _r;
    wind = _w;
    sun = _su;
    snow = _sn;
    humidity = _h;
    pressure = _p;
    return true;
}

bool amedas::getData()
{
    String payload = "";
    HTTPClient http;

    if (http.begin("http://www.jma.go.jp/jp/amedas_h/today-44132.html"))
    {
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            Serial.printf("[http] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                payload = http.getString();
            }
        }
        else
        {
            Serial.printf("[http] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    else
    {
        Serial.printf("[http] Unable to connect\n");
        return false;
    }

    // ある程度整形
    int idxS = payload.indexOf("<div id=\"div_table\"");
    int idxE = payload.indexOf("<!-- end div_table -->");
    String tblData = payload.substring(idxS, idxE);
    tblData.replace("\t", "");
    tblData.replace("&nbsp;", "");
    tblData.replace("</td><td class=\"block middle\">", ",");
    tblData.replace("</td>\n<td class=\"block middle\">", ",");
    tblData.replace("</tr>\n<tr>\n<td class=\"time left\">", "");
    tblData.replace("</td>", "");
    Serial.println(tblData);

    //
    //std::fill(amedasData.begin(), amedasData.end(), 24);
}