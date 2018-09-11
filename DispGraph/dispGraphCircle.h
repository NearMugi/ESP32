#ifndef DISP_GRAPH_CIRCLE_H
#define DISP_GRAPH_CIRCLE_H
#include <NefryDisplay.h>

//円グラフ
class graph_circle {

  public:
    //コンストラクタ
    graph_circle() {
    }

    //グラフクラスの初期化
    void setGraph() {
    }

    //グラフデータの追加
    void addGraphData() {
    }

    //補助線を引く
    void dispArea() {
      //表示領域
      NefryDisplay.drawCircle(64, 32, 30);
      NefryDisplay.fillCircle(64, 32, 10);
    }

    //グラフデータの描画
    void updateGraph() {

    }

  private:
};
#endif
