#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

//切り替えボタン
#define PIN_SW D8

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_GRAPH 100000

//表示する種類
enum GRAPH_TYPE {
  NONE,
  LINE,
  BAR_V,
  BAR_S,
  CIRCLE,
  TURMINAL
};
GRAPH_TYPE dispType = NONE;
GRAPH_TYPE dispTypeBef = NONE;

#define GR_1 0
#define GR_2 1
#define GR_3 2

//グラフ切り替え
bool isChange = false;

//グラフ一時停止
bool isStopGraph = false;

//サンプルデータ
long sampleData[3];
float deg = 0;


//折れ線グラフ
#include "dispGraphLine.h"
#include "dispGraphBar.h"
//static変数を定義
int graph_line::valueSIZE;

//折れ線グラフの領域
#define GRAPH_LINE_POS_X 27
#define GRAPH_LINE_POS_Y 10
#define GRAPH_LINE_LEN_X 100
#define GRAPH_LINE_LEN_Y 50
#define GRAPH_LEN_DPP 10 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

#define VALUE_LINE_MIN 0
#define VALUE_LINE_MAX 1023
#define LINE_PLOT_SIZE (GRAPH_LINE_LEN_X / GRAPH_LEN_DPP) + 1
int x[LINE_PLOT_SIZE];  //x座標
int t[LINE_PLOT_SIZE];  //一定間隔に垂線を引くための配列(垂線の有無)
graph_line grline = graph_line(
                      LOOPTIME_GRAPH,
                      GRAPH_LINE_POS_X,
                      GRAPH_LINE_POS_Y,
                      GRAPH_LINE_LEN_X,
                      GRAPH_LINE_LEN_Y,
                      GRAPH_LEN_DPP,
                      VALUE_LINE_MIN,
                      VALUE_LINE_MAX,
                      LINE_PLOT_SIZE,
                      &x[0],
                      &t[0]
                    );

//折れ線グラフのグラフの設定
//頂点の数が可変なので外部で配列を用意している
int v1[LINE_PLOT_SIZE]; //頂点の値
int v2[LINE_PLOT_SIZE]; //頂点の値
int v3[LINE_PLOT_SIZE]; //頂点の値



//棒グラフ
#include "dispGraphBar.h"
//static変数を定義
int graph_bar::valueSIZE;

//棒グラフ(縦方向)の領域
#define GRAPH_BARV_POS_X 27
#define GRAPH_BARV_POS_Y 20
#define GRAPH_BARV_LEN_X 100
#define GRAPH_BARV_LEN_Y 40
//棒グラフ(横方向)の領域
#define GRAPH_BARS_POS_X 10
#define GRAPH_BARS_POS_Y 20
#define GRAPH_BARS_LEN_X 90
#define GRAPH_BARS_LEN_Y 40

#define VALUE_BAR_MIN 0
#define VALUE_BAR_MAX 1023
#define BAR_PLOT_SIZE 20  //保存するデータ数
graph_bar grbar[2];


//棒グラフのグラフの設定
//保存するデータ数が可変なので外部で配列を用意している
//縦方向と横方向は同じデータを使う
int vb1[BAR_PLOT_SIZE];
int vb2[BAR_PLOT_SIZE];
int vb3[BAR_PLOT_SIZE];


//円グラフ
#include "dispGraphCircle.h"
graph_circle grcir = graph_circle();

void setup() {
  Nefry.enableSW();

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  pinMode(PIN_SW, INPUT_PULLUP);

  //描画するタイプを選択
  dispType = NONE;

  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));
}

//グラフの初期化
void dispGraphLine_init() {
  grline.initGraphTime();
  grline.setGraph(GR_1, &v1[0], VERTEX_CIR, DISP_MAX);
  grline.setGraph(GR_2, &v2[0], VERTEX_NONE, NOTDISP_MAX);
  grline.setGraph(GR_3, &v3[0], VERTEX_NONE, NOTDISP_MAX);
}

void dispGraphBarV_init() {
  grbar[0] = graph_bar(
                     BAR_VERTICAL,
                     LOOPTIME_GRAPH,
                     GRAPH_BARV_POS_X,
                     GRAPH_BARV_POS_Y,
                     GRAPH_BARV_LEN_X,
                     GRAPH_BARV_LEN_Y,
                     VALUE_BAR_MIN,
                     VALUE_BAR_MAX,
                     BAR_PLOT_SIZE
                   );

  grbar[0].initGraphTime();
  grbar[0].setGraph(GR_1, &vb1[0]);
  grbar[0].setGraph(GR_2, &vb2[0]);
  grbar[0].setGraph(GR_3, &vb3[0]);
}

void dispGraphBarS_init() {
  grbar[1] = graph_bar(
                     BAR_SIDE,
                     LOOPTIME_GRAPH,
                     GRAPH_BARS_POS_X,
                     GRAPH_BARS_POS_Y,
                     GRAPH_BARS_LEN_X,
                     GRAPH_BARS_LEN_Y,
                     VALUE_BAR_MIN,
                     VALUE_BAR_MAX,
                     BAR_PLOT_SIZE
                   );
  grbar[1].initGraphTime();
  grbar[1].setGraph(GR_1, &vb1[0]);
  grbar[1].setGraph(GR_2, &vb2[0]);
  grbar[1].setGraph(GR_3, &vb3[0]);
}

void dispGraphCircle_init() {
  //グラフ番号・頂点座標(x,y)・半径(r)・対象の値(最小値,最大値)
  grcir.setGraph(GR_1, 20, 35, 20, 0, 1023);
  grcir.setGraph(GR_2, 62, 35, 20, 0, 1023);
  grcir.setGraph(GR_3, 104, 35, 20, 0, 1023);
}

//折れ線グラフの描画
void dispGraphLine_update() {
  grline.dispArea();
  grline.updateGraph();
}

//棒グラフ(縦方向)の描画
void dispGraphBarV_update() {
  grbar[0].dispArea();
  grbar[0].updateGraph();
}

//棒グラフ(横方向)の描画
void dispGraphBarS_update() {
  grbar[1].dispArea();
  grbar[1].updateGraph();
}

//円グラフの描画
void dispGraphCircle_update() {
  grcir.dispArea();
  grcir.updateGraph();
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  switch (dispType) {
    case LINE:
      dispGraphLine_update();
      break;
    case BAR_V:
      dispGraphBarV_update();
      break;
    case BAR_S:
      dispGraphBarS_update();
      break;
    case CIRCLE:
      dispGraphCircle_update();
      break;
    default:
      NefryDisplay.setFont(ArialMT_Plain_16);
      NefryDisplay.drawString(0, 0, "Select GraphType...");
      break;
  }
  NefryDisplay.display();
}


void loop() {

  //表示グラフの切り替え
  if (isChange) {
    if (!digitalRead(PIN_SW)) {
      dispType = (GRAPH_TYPE)((int)dispType + 1);
      if (dispType == TURMINAL) dispType = LINE;
      isChange = false;
    }
  } else {
    if (digitalRead(PIN_SW)) {
      isChange = true;
    }
  }

  if (dispType != dispTypeBef) {
    switch (dispType) {
      case LINE:
        dispGraphLine_init();
        break;
      case BAR_V:
        dispGraphBarV_init();
        break;
      case BAR_S:
        dispGraphBarS_init();
        break;
      case CIRCLE:
        dispGraphCircle_init();
        break;
    }
  }
  dispTypeBef = dispType;

  //グラフ更新一時停止
  if (Nefry.readSW()) {
    isStopGraph = !isStopGraph;
  }

  //データ・グラフ更新
  interval<LOOPTIME_GRAPH>::run([] {
    if (!isStopGraph) {
      sampleData[0] = random(VALUE_LINE_MIN + 300, VALUE_LINE_MAX + 1 - 200);
      sampleData[1] = (VALUE_LINE_MAX / 2) * (1 + sin(deg / (180 / PI)));
      sampleData[2] = (VALUE_LINE_MAX / 2) * (1 + cos(deg / (180 / PI)));
      deg += 10;

      switch (dispType) {
        case LINE:
          grline.addGraphData(GR_1, (int)sampleData[0]);
          grline.addGraphData(GR_2, (int)sampleData[1]);
          grline.addGraphData(GR_3, (int)sampleData[2]);
          grline.updateGraphTime();
          break;
        case BAR_V:
          grbar[0].addGraphData(GR_1, (int)sampleData[0]);
          grbar[0].addGraphData(GR_2, (int)sampleData[1]);
          grbar[0].addGraphData(GR_3, (int)sampleData[2]);
          grbar[0].updateGraphTime();
          break;
        case BAR_S:
          grbar[1].addGraphData(GR_1, (int)sampleData[0]);
          grbar[1].addGraphData(GR_2, (int)sampleData[1]);
          grbar[1].addGraphData(GR_3, (int)sampleData[2]);
          grbar[1].updateGraphTime();
          break;
        case CIRCLE:
          grcir.addGraphData(GR_1, (int)sampleData[0]);
          grcir.addGraphData(GR_2, (int)sampleData[1]);
          grcir.addGraphData(GR_3, (int)sampleData[2]);
          break;
      }
    }

    DispNefryDisplay();
  });
}
