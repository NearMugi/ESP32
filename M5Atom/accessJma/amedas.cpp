#include "amedas.h"

bool amedas::getData()
{
    String url = "http://www.jma.go.jp/jp/amedas_h/today-" + posID + ".html";
    String payload = "";
    HTTPClient http;

    if (http.begin(url))
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
    tblData.replace("</tr>", "");
    tblData.replace("</table>", "");
    tblData.replace("</div>", "");
    //Serial.println(tblData);

    // 1～24時のデータ(1,6.9,0.0,西北西,2.1,,0,77,1017.4)を保存
    Serial.println("\nGet Data");
    String tmpSplitData = "";
    std::vector<String> tmpTblLineData;
    for (const char &c : tblData)
    {
        if (c == '\n')
        {
            if (std::isdigit(tmpSplitData[0]) != 0)
            {
                Serial.println(tmpSplitData);
                tmpTblLineData.push_back(tmpSplitData);
            }
            tmpSplitData = "";
        }
        else
        {
            tmpSplitData += c;
        }
    }

    // カンマで区切ってデータを保存
    for (const String &t : tmpTblLineData)
    {
        tmpSplitData = "";
        std::vector<String> tmpTblData;
        for (const char &c : t)
        {
            if (c == ',')
            {
                //Serial.println(tmpSplitData);
                tmpTblData.push_back(tmpSplitData);
                tmpSplitData = "";
            }
            else
            {
                tmpSplitData += c;
            }
        }
        if (std::isdigit(tmpSplitData[0]) != 0)
        {
            //Serial.println(tmpSplitData);
            tmpTblData.push_back(tmpSplitData);
        }

        // 先頭のデータを削除
        tmpTblData.erase(tmpTblData.begin());

        // 先頭の値(気温)が数値の場合、データを保存する
        if (tmpTblData[0].length() > 0)
        {
            _tblData.emplace_back(
                unitAmedasData(
                    tmpTblData[0].toFloat(),
                    tmpTblData[1].toFloat(),
                    tmpTblData[2],
                    tmpTblData[3].toFloat(),
                    tmpTblData[4].toFloat(),
                    tmpTblData[5].toFloat(),
                    tmpTblData[6].toFloat(),
                    tmpTblData[7].toFloat()));
        }
    }

    Serial.println("\nTable Size");
    Serial.println(_tblData.size());
    _lastData = _tblData[_tblData.size() - 1];
    Serial.println("\nLast Data");
    Serial.println(_lastData.getTemperature());
    Serial.println(_lastData.getRain());
    Serial.println(_lastData.getWindDirection());
    Serial.println(_lastData.getWind());
    Serial.println(_lastData.getSun());
    Serial.println(_lastData.getSnow());
    Serial.println(_lastData.getHumidity());
    Serial.println(_lastData.getPressure());
}