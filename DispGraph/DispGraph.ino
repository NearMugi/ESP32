#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_GRAPH 100000

//グラフ一時停止
bool isStopGraph = false;

//サンプルデータ
long sampleData[3];
float deg = 0;


//折れ線グラフ
#include "dispGraphLine.h"
#include "dispGraphBar.h"
//static変数を定義
int graph_line::lpTime;
int graph_line::posX;
int graph_line::posY;
int graph_line::lenX;
int graph_line::lenY;
int graph_line::dpp;
int graph_line::valueMIN;
int graph_line::valueMAX;
int graph_line::valueSIZE;

//折れ線グラフの領域
#define GRAPH_LINE_POS_X 27
#define GRAPH_LINE_POS_Y 10
#define GRAPH_LINE_LEN_X 100
#define GRAPH_LINE_LEN_Y 50
#define GRAPH_LEN_DPP 10 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

#define VALUE_LINE_MIN 0
#define VALUE_LINE_MAX 1023
#define VALUE_LINE_SIZE (GRAPH_LINE_LEN_X / GRAPH_LEN_DPP) + 1
int x[VALUE_LINE_SIZE];  //x座標
int t[VALUE_LINE_SIZE];  //一定間隔に垂線を引くための配列(垂線の有無)
graph_line grline = graph_line(
                      LOOPTIME_GRAPH,
                      GRAPH_LINE_POS_X,
                      GRAPH_LINE_POS_Y,
                      GRAPH_LINE_LEN_X,
                      GRAPH_LINE_LEN_Y,
                      GRAPH_LEN_DPP,
                      VALUE_LINE_MIN,
                      VALUE_LINE_MAX,
                      VALUE_LINE_SIZE,
                      &x[0],
                      &t[0]
                    );

//折れ線グラフのグラフの設定
//頂点の数が可変なので外部で配列を用意している
#define GL_1 0
#define GL_2 1
#define GL_3 2
int v1[VALUE_LINE_SIZE]; //頂点の値
int p1[VALUE_LINE_SIZE]; //グラフでプロットするy座標
int v2[VALUE_LINE_SIZE]; //頂点の値
int p2[VALUE_LINE_SIZE]; //グラフでプロットするy座標
int v3[VALUE_LINE_SIZE]; //頂点の値
int p3[VALUE_LINE_SIZE]; //グラフでプロットするy座標



//棒グラフ
#include "dispGraphBar.h"
//static変数を定義
int graph_bar::lpTime;
int graph_bar::posX[2];
int graph_bar::posY[2];
int graph_bar::lenX[2];
int graph_bar::lenY[2];
int graph_bar::valueMIN;
int graph_bar::valueMAX;
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
#define VALUE_BAR_SIZE 20  //保存するデータ数
graph_bar grbarV = graph_bar(
                     BAR_VERTICAL,
                     LOOPTIME_GRAPH,
                     GRAPH_BARV_POS_X,
                     GRAPH_BARV_POS_Y,
                     GRAPH_BARV_LEN_X,
                     GRAPH_BARV_LEN_Y,
                     VALUE_BAR_MIN,
                     VALUE_BAR_MAX,
                     VALUE_BAR_SIZE
                   );
graph_bar grbarS = graph_bar(
                     BAR_SIDE,
                     LOOPTIME_GRAPH,
                     GRAPH_BARS_POS_X,
                     GRAPH_BARS_POS_Y,
                     GRAPH_BARS_LEN_X,
                     GRAPH_BARS_LEN_Y,
                     VALUE_BAR_MIN,
                     VALUE_BAR_MAX,
                     VALUE_BAR_SIZE
                   );

//棒グラフのグラフの設定
//保存するデータ数が可変なので外部で配列を用意している
//縦方向と横方向は同じデータを使う
//そのため最大値・平均値も外部に定義した
int vb1[VALUE_BAR_SIZE];
int vb2[VALUE_BAR_SIZE];
int vb3[VALUE_BAR_SIZE];
int vMax[3];
int vAve[3];
void setup() {
  Nefry.enableSW();

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  //表示するグラフの設定
  grline.setGraph(GL_1, &v1[0], &p1[0], VERTEX_CIR, DISP_MAX);
  grline.setGraph(GL_2, &v2[0], &p2[0], VERTEX_NONE, NOTDISP_MAX);
  grline.setGraph(GL_3, &v3[0], &p3[0], VERTEX_NONE, NOTDISP_MAX);

  grbarV.setGraph(GL_1, &vb1[0], &vMax[0], &vAve[0]);
  grbarV.setGraph(GL_2, &vb2[0], &vMax[1], &vAve[1]);
  grbarV.setGraph(GL_3, &vb3[0], &vMax[2], &vAve[2]);

  grbarS.setGraph(GL_1, &vb1[0], &vMax[0], &vAve[0]);
  grbarS.setGraph(GL_2, &vb2[0], &vMax[1], &vAve[1]);
  grbarS.setGraph(GL_3, &vb3[0], &vMax[2], &vAve[2]);

  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));
}

//折れ線グラフの描画
void dispGraphLine_update() {
  grline.dispArea();
  grline.updateGraph();
}

//棒グラフ(縦方向)の描画
void dispGraphBarV_update() {
  grbarV.dispArea();
  grbarV.updateGraph();
}

//棒グラフ(横方向)の描画
void dispGraphBarS_update() {
  grbarS.dispArea();
  grbarS.updateGraph();
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  //dispGraphLine_update();
  dispGraphBarV_update();
  //dispGraphBarS_update();
  NefryDisplay.display();
}


void loop() {
  if (Nefry.readSW()) {
    isStopGraph = !isStopGraph;
  }

  interval<LOOPTIME_GRAPH>::run([] {
    if (!isStopGraph) {
      sampleData[0] = random(VALUE_LINE_MIN + 100, VALUE_LINE_MAX + 1 - 100);
      sampleData[1] = (VALUE_LINE_MAX / 2) * (1 + sin(deg / (180 / PI)));
      sampleData[2] = (VALUE_LINE_MAX / 2) * (1 + cos(deg / (180 / PI)));
      deg += 10;

      grline.addGraphData(GL_1, (int)sampleData[0]);
      grline.addGraphData(GL_2, (int)sampleData[1]);
      grline.addGraphData(GL_3, (int)sampleData[2]);
      grline.updateGraphTime();

      grbarV.addGraphData(GL_1, (int)sampleData[0]);
      grbarV.addGraphData(GL_2, (int)sampleData[1]);
      grbarV.addGraphData(GL_3, (int)sampleData[2]);
      grbarV.updateGraphTime();
    }

    DispNefryDisplay();
  });
}
