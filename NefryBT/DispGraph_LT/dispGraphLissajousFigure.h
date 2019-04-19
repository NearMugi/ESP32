#ifndef DISP_GRAPH_LISSAJOUS_H
#define DISP_GRAPH_LISSAJOUS_H
#include <NefryDisplay.h>

//リサージュ
class graph_lissajous {

  public:
    //コンストラクタ
    graph_lissajous(){};
    graph_lissajous(int _posX,
                    int _posY,
                    int _lenX,
                    int _lenY,
                    int _valueMIN,
                    int _valueMAX,
                    int _valueSIZE
                   ) {
      posX = _posX;
      posY = _posY;
      lenX = _lenX;
      lenY = _lenY;
      lenX_Half = _lenX / 2;
      lenY_Half = _lenY / 2;
      valueMIN = _valueMIN;
      valueMAX = _valueMAX;
      valueSIZE = _valueSIZE;

    }
    //グラフクラスの初期化
    void setGraph(float *_p) {
      _g.init(&_p[0]);
    }

    //グラフデータの追加
    void addGraphData(float _x, float _y) {
      if (!_g.isSet) return;
      _g.addData(_x, _y);
    }


    //補助線を引く
    void dispArea() {
      //表示領域
      NefryDisplay.drawVerticalLine(posX + lenX_Half, posY, lenY);
      NefryDisplay.drawHorizontalLine(posX, posY + lenY_Half, lenX);
    }

    //グラフデータの描画
    void updateGraph() {
        if (!_g.isSet) return;

        for (int i = 0; i < valueSIZE - 1; i++) {
          //グラフの座標に合わせて値を変換する
          int x0 = (*(_g.p + i * 2) - valueMIN) * (float)lenX / (float)(valueMAX - valueMIN) + posX;
          int y0 = (*(_g.p + i * 2 + 1) - valueMIN) * (float)lenY / (float)(valueMAX - valueMIN) + posY;
          
          int x1 = (*(_g.p + (i + 1) * 2) - valueMIN) * (float)lenX / (float)(valueMAX - valueMIN) + posX;
          int y1 = (*(_g.p + (i + 1) * 2 + 1) - valueMIN) * (float)lenY / (float)(valueMAX - valueMIN) + posY;

          NefryDisplay.drawLine(x0, y0, x1, y1); //各座標を線で結ぶ
        }
    }

  private:
    //グラフ領域
    int posX;
    int posY;
    int lenX;
    int lenX_Half;
    int lenY;
    int lenY_Half;
    int valueMIN;
    int valueMAX;
    static int valueSIZE;

    //描画するグラフの値・頂点などの情報
    class graph {
      public:
        bool isSet;
        float *p; //プロットするデータ(変換前)

        //コンストラクタ
        graph() {
          isSet = false;
        }

        void init(float *_p) {
          isSet = true;
          p = _p;

          for (int i = 0; i < valueSIZE; i++) {
            *(p + i * 2) = 0;
            *(p + i * 2 + 1) = 0;
          }
        }

        void addData(float _x, float _y) {          
          //前にずらす
          for (int i = 0; i < valueSIZE - 1; i++) {
            *(p + i * 2) = *(p + (i + 1) * 2);
            *(p + i * 2 + 1) = *(p + (i + 1) * 2 + 1);
          }
          //最後尾に追加する
          *(p + (valueSIZE - 1) * 2) = _x;
          *(p + (valueSIZE - 1) * 2 + 1) = _y;
        }
    };
    graph _g;
};
#endif
