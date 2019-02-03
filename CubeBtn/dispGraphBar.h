#ifndef DISP_GRAPH_BAR_H
#define DISP_GRAPH_BAR_H
#include <NefryDisplay.h>

//表示するグラフの最大数
#define GRAPH_BAR_CNT 3

//棒グラフの向き
enum GRAPH_BAR_TYPE {
  BAR_VERTICAL = 0, //縦方向
  BAR_SIDE = 1, //横方向
};

//棒グラフ
class graph_bar {

  public:
    //コンストラクタ
    graph_bar(){}
    graph_bar(GRAPH_BAR_TYPE _type,
              int _lpTime,
              int _posX,
              int _posY,
              int _lenX,
              int _lenY,
              int _valueMIN,
              int _valueMAX,
              int _valueSIZE
             ) {
      type = _type;
      lpTime = _lpTime;
      posX = _posX;
      posY = _posY;
      lenX = _lenX;
      lenY = _lenY;
      valueMIN = _valueMIN;
      valueMAX = _valueMAX;
      valueSIZE = _valueSIZE;

    }

    //グラフクラスの初期化
    void setGraph(int _idx, int *_v) {
      _g[_idx].init(&_v[0]);
    }

    //グラフデータの追加
    void addGraphData(int _idx, int _v) {
      if (!_g[_idx].isSet) return;
      _g[_idx].addData(_v);
    }
    
    //時間の初期化
    void initGraphTime(){
      tTotal = 0;
      tCntMax = 1000000 / lpTime; //1秒カウント   
    }
    //時間の更新
    void updateGraphTime() {
      if (tCntMax > 0) {
        tCnt = (tCnt + 1) % tCntMax;
        if (tCnt == 0) {
          tTotal++;
        }
      }
    }

    //補助線を引く
    void dispArea() {
      //表示領域
      switch (type) {
        case BAR_VERTICAL:
          NefryDisplay.drawHorizontalLine(posX, posY, lenX);
          NefryDisplay.drawHorizontalLine(posX, posY + lenY, lenX);
          NefryDisplay.setFont(ArialMT_Plain_10);
          NefryDisplay.drawString(posX - 22, 0, "Max:");
          NefryDisplay.drawString(posX - 22, 8, "Ave:");

          NefryDisplay.drawString(posX - 22, posY, String(valueMAX));
          NefryDisplay.drawString(posX - 22, posY + lenY - 8, String(valueMIN));

          break;

        case BAR_SIDE:
          //NefryDisplay.drawVerticalLine(posX, posY, lenY);
          //NefryDisplay.drawVerticalLine(posX + lenX, posY, lenY);
          //NefryDisplay.setFont(ArialMT_Plain_10);
          //NefryDisplay.drawString(posX + lenX + 2, 0, "Max");
          //NefryDisplay.drawString(posX + lenX + 2, 8, "Ave");

          //NefryDisplay.drawString(posX + lenX - 22, 5, String(valueMAX));
          //NefryDisplay.drawString(posX, 5, String(valueMIN));
          break;
      }

    }

    //グラフデータの描画
    void updateGraph() {
      for (int _idx = 0; _idx < GRAPH_BAR_CNT; _idx++) {
        if (!_g[_idx].isSet) continue;
        int ofs = 0;
        int p = 0;
        switch (type) {
          case BAR_VERTICAL:
            switch (_idx) {
              case 0:
                ofs = 18;
                break;
              case 1:
                ofs = 49;
                break;
              case 2:
                ofs = 80;
                break;
            }

            //最大値と平均値
            p = map(_g[_idx].vMax, valueMIN, valueMAX, lenY + posY, posY);
            NefryDisplay.fillRect(posX + ofs - 13, p, 26, 2);
            NefryDisplay.setFont(ArialMT_Plain_10);
            NefryDisplay.drawString(posX + ofs - 15, 0, String(_g[_idx].vMax));
            NefryDisplay.drawString(posX + ofs - 15, 8, String(_g[_idx].vAve));

            //値
            p = map(*(_g[_idx].v + valueSIZE - 1), valueMIN, valueMAX, lenY + posY, posY);
            NefryDisplay.fillRect(posX + ofs - 13, p, 26, posY + lenY - p);

            break;

          case BAR_SIDE:
            switch (_idx) {
              case 0:
                ofs = 2;
                break;
              case 1:
                ofs = 16;
                break;
              case 2:
                ofs = 30;
                break;
            }

            //最大値と平均値
            //p = map(_g[_idx].vMax, valueMIN, valueMAX, posX, lenX + posX);
            //NefryDisplay.fillRect(p, posY + ofs, 2, 10);
            //NefryDisplay.setFont(ArialMT_Plain_10);
            //NefryDisplay.drawString(posX + lenX + 2, posY + ofs - 5, String(_g[_idx].vMax));
            //NefryDisplay.drawString(posX + lenX + 2, posY + ofs + 3, String(_g[_idx].vAve));

            //値
            p = map(*(_g[_idx].v + valueSIZE - 1), valueMIN, valueMAX, 0, lenX);
            NefryDisplay.fillRect(posX + p, posY, 5, 10);
            break;
        }
      }
    }

  private:
    GRAPH_BAR_TYPE type;
    //グラフ領域・ピッチ
    int lpTime;
    int posX;
    int posY;
    int lenX;
    int lenY;
    int valueMIN;
    int valueMAX;
    static int valueSIZE;

    //時間
    int tTotal;
    int tCntMax;
    int tCnt;

    //描画するグラフの値などの情報
    class graph {
      public:
        bool isSet;
        int *v; //頂点の値
        int vMax;  //最大値
        int vAve;  //平均値

        //コンストラクタ
        graph() {
          isSet = false;
        }

        void init(int *_v) {
          isSet = true;
          v = _v;
          vMax = 0;
          vAve = 0;

          for (int i = 0; i < valueSIZE; i++) {
            *(v + i) = 0;
          }
        }

        void addData(int _v) {
          vMax = _v;  //最大値に仮設定
          vAve = 0;
          //前にずらす
          for (int i = 0; i < valueSIZE - 1; i++) {
            *(v + i) = *(v + i + 1);
            if (*(v + i) > vMax) vMax = *(v + i);
            vAve += *(v + i);
          }
          //最後尾に追加する
          *(v + valueSIZE - 1) = _v;
          vAve += *(v + valueSIZE - 1);
          vAve /= valueSIZE;
        }
    };
    graph _g[GRAPH_BAR_CNT];
};
#endif
