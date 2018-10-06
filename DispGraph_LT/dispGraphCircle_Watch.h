//円グラフを時計にしたもの
#ifndef DISP_GRAPH_CIRCLE_WATCH_H
#define DISP_GRAPH_CIRCLE_WATCH_H
#include <NefryDisplay.h>

//表示する針の数
#define GRAPH_WATCH_CNT 3
#define CL_HOUR 0
#define CL_MIN 1
#define CL_SEC 2
//左上の座標と円サイズ
#define CL_X 32
#define CL_Y 32
#define CL_R 30

//時計
class graph_circle_watch {

  public:
    //コンストラクタ
    graph_circle_watch() {
    }

    //グラフクラスの初期化
    void setGraph(int _idx, int x, int y, int _r, int _valueMin, int _valueMax, bool _isFill) {
      _g[_idx].init( x, y, _r, _valueMin, _valueMax, _isFill);
    }

    //グラフデータの追加
    void addGraphData(int _idx, int _v) {
      _g[_idx].setDeg(_v);
    }

    //グラフデータの描画
    void updateGraph() {
      float rad = 0.0;
      float _x = 0.0;
      float _y = 0.0;
      for (int _idx = 0; _idx < GRAPH_WATCH_CNT; _idx++) {
        if (!_g[_idx].isSet) continue;

        //中を線で塗りつぶし
        if (_g[_idx].isFill) {
          //角度を90度ずらしているので開始は-90から。
          for (int i = -90; i < _g[_idx].deg; i = i + 12) {
            rad = i * PI / 180;
            _x = _g[_idx].p[0] + (float)_g[_idx].r * cos(rad);
            _y = _g[_idx].p[1] + (float)_g[_idx].r * sin(rad);
            NefryDisplay.drawLine(_g[_idx].p[0], _g[_idx].p[1], (int)_x, (int)_y);
          }
        }

        //対象の値
        rad = _g[_idx].deg * PI / 180;
        _x = _g[_idx].p[0] + (float)_g[_idx].r * cos(rad);
        _y = _g[_idx].p[1] + (float)_g[_idx].r * sin(rad);
        NefryDisplay.drawLine(_g[_idx].p[0], _g[_idx].p[1], (int)_x, (int)_y);

        //秒針は枠に丸ぽちをつける
        if (_idx == CL_SEC) {
          _x = _g[_idx].p[0] + (float)CL_R * cos(rad);
          _y = _g[_idx].p[1] + (float)CL_R * sin(rad);
          NefryDisplay.fillRect((int)_x - 2, (int)_y - 2, 4, 4);
        }

      }



    }

  private:
    //描画するグラフの値などの情報
    class graph {
      public:
        bool isSet;
        bool isFill;  //塗りつぶしかどうか
        int p[2]; //頂点の位置
        int r; //半径
        int value;
        int valueMin;  //取りうる値の最小値
        int valueMax;  //取りうる値の最大値
        float deg;  //値の割合をもとに変換した角度
        //コンストラクタ
        graph() {
          isSet = false;
        }

        void init(int x, int y, int _r, int _valueMin, int _valueMax, bool _isFill) {
          isSet = true;
          isFill = _isFill;
          p[0] = x;
          p[1] = y;
          r = _r;
          valueMin = _valueMin;
          valueMax = _valueMax;
          deg = 0;
        }

        void setDeg(int _v) {
          if (valueMax == valueMin) return;
          value = _v;
          deg = (float)(value - valueMin) * (float)360 / (valueMax - valueMin) - 90; //位相
        }
    };
    graph _g[GRAPH_WATCH_CNT];
};
#endif
