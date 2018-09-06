#ifndef DISP_GRAPH_LINE_H
#define DISP_GRAPH_LINE_H
#include <NefryDisplay.h>

//頂点タイプ
enum VERTEX {
  VERTEX_NONE, //頂点そのまま
  VERTEX_CIR,  //丸
  VERTEX_SQU,  //四角
};

//最大値の表示有無
#define DISP_MAX true
#define NOTDISP_MAX false

//表示するグラフの最大数
#define GRAPH_CNT 3

//折れ線グラフ
class graph_line {

  public:
    //コンストラクタ
    graph_line(int _lpTime,
               int _posX,
               int _posY,
               int _lenX,
               int _lenY,
               int _dpp,
               int _valueMIN,
               int _valueMAX,
               int _valueSIZE,
               int *_x,
               int *_t
              ) {
      lpTime = _lpTime;
      posX = _posX;
      posY = _posY;
      lenX = _lenX;
      lenY = _lenY;
      dpp = _dpp;
      valueMIN = _valueMIN;
      valueMAX = _valueMAX;
      valueSIZE = _valueSIZE;

      //x座標はこれ以降更新しない。
      x = _x;
      for (int i = 0; i < valueSIZE; i++) {
        *(x + i) = posX + i * dpp;
      }

      //時間
      int tTotal;
      tCntMax = 1000000 / lpTime; //1秒ごとに垂線を引く
      t = _t;
      for (int i = 0; i < valueSIZE; i++) {
        *(t + i) = 0;
      }

    }

    //グラフクラスの初期化
    void setGraph(int _idx, int *_v, int *_p, VERTEX _type, bool _isDispMax) {
      _g[_idx].init(&_v[0], &_p[0], _type, _isDispMax);
    }

    //グラフデータの追加
    void addGraphData(int _idx, int _v) {
      if (!_g[_idx].isSet) return;
      _g[_idx].addData(_v);
    }

    //時間の更新
    void updateGraphTime() {
      bool isSplit = false;
      if (tCntMax > 0) {
        tCnt = (tCnt + 1) % tCntMax;
        if (tCnt == 0) {
          isSplit = true;
          tTotal++;
        }
      }

      //前にずらす
      for (int i = 0; i < valueSIZE - 1; i++) {
        *(t + i) = *(t + i + 1);
      }
      //最後尾に追加する
      *(t + valueSIZE - 1) = (int)isSplit;
    }

    //補助線を引く
    void dispArea() {
      //表示領域
      NefryDisplay.drawHorizontalLine(posX, posY, lenX);
      NefryDisplay.drawHorizontalLine(posX, posY + lenY, lenX);
      NefryDisplay.setFont(ArialMT_Plain_10);
      NefryDisplay.drawString(posX - 22, posY - 8, String(valueMAX));
      NefryDisplay.drawString(posX - 22, posY + lenY - 8, String(valueMIN));

      //時間
      NefryDisplay.drawString(posX + lenX - 18, 0, String(tTotal));
      for (int i = 0; i < valueSIZE; i++) {
        if (*(t + i) > 0) {
          NefryDisplay.drawVerticalLine(*(x + i), posY, lenY);
        } else {
          NefryDisplay.drawVerticalLine(*(x + i), posY, 3);
          NefryDisplay.drawVerticalLine(*(x + i), posY + lenY - 3, 3);
        }
      }
    }

    //グラフデータの描画
    void updateGraph() {
      for (int _idx = 0; _idx < GRAPH_CNT; _idx++) {
        if (!_g[_idx].isSet) continue;

        //最大値
        if (_g[_idx].isDispMax) {
          NefryDisplay.drawHorizontalLine(posX, _g[_idx].vMax[1], lenX);
          NefryDisplay.setFont(ArialMT_Plain_10);
          NefryDisplay.drawString(posX - 20, _g[_idx].vMax[1] - 8, String(_g[_idx].vMax[0]));
        }

        for (int i = 0; i < valueSIZE - 1; i++) {

          NefryDisplay.drawLine(*(x + i), *(_g[_idx].p + i),
                                *(x + i + 1), *(_g[_idx].p + i + 1)); //各座標を線で結ぶ
          //各座標をプロットする
          vertexPlot(_g[_idx].type, *(x + i), *(_g[_idx].p + i));
        }
        //最後の点をプロット
        vertexPlot(_g[_idx].type, *(x + valueSIZE - 1), *(_g[_idx].p + valueSIZE - 1));
      }
    }
    void vertexPlot(VERTEX type, int x, int y) {
      switch (type) {
        case VERTEX_NONE:
          break;
        case VERTEX_CIR://頂点を丸くする
          NefryDisplay.fillCircle(x, y, 2);
          break;
        case VERTEX_SQU://頂点を四角くする
          NefryDisplay.fillRect(x - 2, y - 2, 4, 4);
          break;
      }
    }

  private:
    //グラフ領域・ピッチ
    static int lpTime;
    static int posX;
    static int posY;
    static int lenX;
    static int lenY;
    static int dpp;//点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)
    static int valueMIN;
    static int valueMAX;
    static int valueSIZE;
    int *x;
    //時間
    int tTotal;
    int tCntMax;
    int tCnt;
    int *t;

    //描画するグラフの値・頂点などの情報
    class graph {
      public:
        bool isSet;
        bool isDispMax;
        VERTEX type; //頂点のタイプ
        int *v; //頂点の値
        int *p; //プロットする頂点座標
        int vMax[2];  //最大値,平行線のy座標

        //コンストラクタ
        graph() {
          isSet = false;
          isDispMax = false;
        }

        void init(int *_v, int *_p, VERTEX _type, bool _isDispMax) {
          isSet = true;
          isDispMax = _isDispMax;
          type = _type;
          v = _v;
          p = _p;

          for (int i = 0; i < valueSIZE; i++) {
            *(v + i) = 0;
            *(p + i)  = lenY + posY;
          }
        }

        void addData(int _v) {
          vMax[0] = _v;  //最大値に仮設定
          //前にずらす
          for (int i = 0; i < valueSIZE - 1; i++) {
            *(v + i) = *(v + i + 1);
            *(p + i) = *(p + i + 1);
            if (*(v + i) > vMax[0]) vMax[0] = *(v + i);
          }
          //最後尾に追加する
          *(v + valueSIZE - 1) = _v;
          *(p + valueSIZE - 1) = map(_v, valueMIN, valueMAX, lenY + posY, posY);
          vMax[1] = map(vMax[0], valueMIN, valueMAX, lenY + posY, posY);
        }
    };
    graph _g[GRAPH_CNT];
};
#endif
