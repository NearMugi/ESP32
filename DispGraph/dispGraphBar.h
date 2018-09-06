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
      posX[type] = _posX;
      posY[type] = _posY;
      lenX[type] = _lenX;
      lenY[type] = _lenY;
      valueMIN = _valueMIN;
      valueMAX = _valueMAX;
      valueSIZE = _valueSIZE;

      //時間
      tTotal = 0;
      tCntMax = 1000000 / lpTime; //1秒カウント

    }

    //グラフクラスの初期化
    void setGraph(int _idx, int *_v, int *_vMax, int *_vAve) {
      _g[_idx].init(&_v[0], _vMax, _vAve);
    }

    //グラフデータの追加
    void addGraphData(int _idx, int _v) {
      if (!_g[_idx].isSet) return;
      _g[_idx].addData(_v);
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
          NefryDisplay.drawHorizontalLine(posX[type], posY[type], lenX[type]);
          NefryDisplay.drawHorizontalLine(posX[type], posY[type] + lenY[type], lenX[type]);
          NefryDisplay.setFont(ArialMT_Plain_10);
          NefryDisplay.drawString(posX[type] - 22, 0, "Max:");
          NefryDisplay.drawString(posX[type] - 22, 8, "Ave:");

          NefryDisplay.drawString(posX[type] - 22, posY[type], String(valueMAX));
          NefryDisplay.drawString(posX[type] - 22, posY[type] + lenY[type] - 8, String(valueMIN));

          break;

        case BAR_SIDE:
          NefryDisplay.drawVerticalLine(posX[type], posY[type], lenY[type]);
          NefryDisplay.drawVerticalLine(posX[type] + lenX[type], posY[type], lenY[type]);
          NefryDisplay.setFont(ArialMT_Plain_10);
          NefryDisplay.drawString(posX[type] + lenX[type] + 2, 0, "Max");
          NefryDisplay.drawString(posX[type] + lenX[type] + 2, 8, "Ave");

          NefryDisplay.drawString(posX[type] + lenX[type] - 22, 5, String(valueMAX));
          NefryDisplay.drawString(posX[type], 5, String(valueMIN));
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
            p = map(*_g[_idx].vMax, valueMIN, valueMAX, lenY[type] + posY[type], posY[type]);
            NefryDisplay.fillRect(posX[type] + ofs - 13, p, 26, 2);
            NefryDisplay.setFont(ArialMT_Plain_10);
            NefryDisplay.drawString(posX[type] + ofs - 15, 0, String(*_g[_idx].vMax));
            NefryDisplay.drawString(posX[type] + ofs - 15, 8, String(*_g[_idx].vAve));

            //値
            p = map(*(_g[_idx].v + valueSIZE - 1), valueMIN, valueMAX, lenY[type] + posY[type], posY[type]);
            NefryDisplay.fillRect(posX[type] + ofs - 13, p, 26, posY[type] + lenY[type] - p);

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
            p = map(*_g[_idx].vMax, valueMIN, valueMAX, posX[type], lenX[type] + posX[type]);
            NefryDisplay.fillRect(p, posY[type] + ofs, 2, 10);
            NefryDisplay.setFont(ArialMT_Plain_10);
            NefryDisplay.drawString(posX[type] + lenX[type] + 2, posY[type] + ofs - 5, String(*_g[_idx].vMax));
            NefryDisplay.drawString(posX[type] + lenX[type] + 2, posY[type] + ofs + 3, String(*_g[_idx].vAve));

            //値
            p = map(*(_g[_idx].v + valueSIZE - 1), valueMIN, valueMAX, 0, lenX[type]);
            NefryDisplay.fillRect(posX[type], posY[type] + ofs, p, 10);
            break;
        }
      }
    }

  private:
    GRAPH_BAR_TYPE type;
    //グラフ領域・ピッチ
    //※領域は縦方向・横方向の値を別々に保存する。static変数の使用による支障
    static int lpTime;
    static int posX[2];
    static int posY[2];
    static int lenX[2];
    static int lenY[2];
    static int valueMIN;
    static int valueMAX;
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
        int *vMax;  //最大値
        int *vAve;  //平均値

        //コンストラクタ
        graph() {
          isSet = false;
        }

        void init(int *_v, int *_vMax, int *_vAve) {
          isSet = true;
          v = _v;
          vMax = _vMax;
          vAve = _vAve;

          for (int i = 0; i < valueSIZE; i++) {
            *(v + i) = 0;
          }
          *vMax = 0;
          *vAve = 0;
        }

        void addData(int _v) {
          *vMax = _v;  //最大値に仮設定
          *vAve = 0;
          //前にずらす
          for (int i = 0; i < valueSIZE - 1; i++) {
            *(v + i) = *(v + i + 1);
            if (*(v + i) > *vMax) *vMax = *(v + i);
            *vAve += *(v + i);
          }
          //最後尾に追加する
          *(v + valueSIZE - 1) = _v;
          *vAve += *(v + valueSIZE - 1);
          *vAve /= valueSIZE;
        }
    };
    graph _g[GRAPH_BAR_CNT];
};
#endif
