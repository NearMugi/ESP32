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
#define GRAPH_LINE_CNT 3

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
      t = _t;

    }
    //グラフクラスの初期化
    void setGraph(int _idx, int *_v, VERTEX _type, bool _isDispMax) {
      _g[_idx].init(&_v[0], _type, _isDispMax);
    }

    //グラフデータの追加
    void addGraphData(int _idx, int _v) {
      if (!_g[_idx].isSet) return;
      _g[_idx].addData(_v);
    }
    
    //時間の初期化
    void initGraphTime(){
      tCntMax = 1000000 / lpTime; //1秒ごとに垂線を引く
      tCnt = 0;
      tTotal = 0;
      for (int i = 0; i < valueSIZE; i++) {
        *(t + i) = 0;
      }      
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
      for (int _idx = 0; _idx < GRAPH_LINE_CNT; _idx++) {
        if (!_g[_idx].isSet) continue;

        //最大値
        int _tmpMax = lenY + posY;
        if (_g[_idx].isDispMax) {
          _tmpMax = map(_g[_idx].vMax, valueMIN, valueMAX, lenY + posY, posY);
          NefryDisplay.drawHorizontalLine(posX, _tmpMax, lenX);
          NefryDisplay.setFont(ArialMT_Plain_10);
          NefryDisplay.drawString(posX - 20, _tmpMax - 8, String(_g[_idx].vMax));
        }
        
        //各座標を線で結ぶ  
        int y0 = lenY + posY;
        int y1 = lenY + posY;        
        for (int i = 0; i < valueSIZE - 1; i++) {
          y0 = map(*(_g[_idx].v + i), valueMIN, valueMAX, lenY + posY, posY);
          y1 = map(*(_g[_idx].v + i + 1), valueMIN, valueMAX, lenY + posY, posY);
          NefryDisplay.drawLine(*(x + i), y0, *(x + i + 1), y1); 
          //各座標をプロットする
          vertexPlot(_g[_idx].type, *(x + i), y0);
        }
        //最後の点をプロット
        y0 = map(*(_g[_idx].v + valueSIZE - 1), valueMIN, valueMAX, lenY + posY, posY);
        vertexPlot(_g[_idx].type, *(x + valueSIZE - 1), y0);
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
    int lpTime;
    int posX;
    int posY;
    int lenX;
    int lenY;
    int dpp;//点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)
    int valueMIN;
    int valueMAX;
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
        int vMax;  //最大値

        //コンストラクタ
        graph() {
          isSet = false;
          isDispMax = false;
        }

        void init(int *_v,VERTEX _type, bool _isDispMax) {
          isSet = true;
          isDispMax = _isDispMax;
          type = _type;
          v = _v;

          for (int i = 0; i < valueSIZE; i++) {
            *(v + i) = 0;
          }
        }

        void addData(int _v) {
          vMax = _v;  //最大値に仮設定
          //前にずらす
          for (int i = 0; i < valueSIZE - 1; i++) {
            *(v + i) = *(v + i + 1);
            if (*(v + i) > vMax) vMax = *(v + i);
          }
          //最後尾に追加する
          *(v + valueSIZE - 1) = _v;
        }
    };
    graph _g[GRAPH_LINE_CNT];
};
#endif
