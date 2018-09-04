#ifndef DISP_GRAPH_H
#define DISP_GRAPH_H
#include <NefryDisplay.h>

//頂点タイプ
#define VERTEX_NONE 0  //頂点そのまま
#define VERTEX_CIR 1  //丸
#define VERTEX_SQU 2  //四角

//折れ線グラフ
class graph_line {

  public:
    //グラフ領域・ピッチ・更新時間
    int lpTime;
    int posX;
    int posY;
    int lenX;
    int lenY;
    int dpp;//点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)
    int valueMIN;
    int valueMAX;
    int valueSIZE;

    //コンストラクタ
    graph_line(int _lpTime, int _posX, int _posY, int _lenX, int _lenY, int _dpp, int _valueMIN, int _valueMAX, int _valueSIZE) {
      lpTime = _lpTime;
      posX = _posX;
      posY = _posY;
      lenX = _lenX;
      lenY = _lenY;
      dpp = _dpp;
      valueMIN = _valueMIN;
      valueMAX = _valueMAX;
      valueSIZE = _valueSIZE;
    }


    void dispArea() {
      //補助線
      NefryDisplay.drawHorizontalLine(posX, posY, lenX);
      NefryDisplay.drawHorizontalLine(posX, posY + lenY, lenX);
      NefryDisplay.setFont(ArialMT_Plain_10);
      NefryDisplay.drawString(posX - 20, posY - 8, String(valueMAX));
      NefryDisplay.drawString(posX - 20, posY + lenY - 8, String(valueMIN));
    }

  private:
      int p[128][2];



};

#endif
