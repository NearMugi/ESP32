#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

//ループ周期(us)
#include <interval.h>

//グラフ一時停止
bool isStopGraph = false;

//サンプルデータ
long sampleData[3];
float deg = 0;


//グラフ
#include "dispGraph.h"
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
#define LOOPTIME_GRAPH 100000
#define GRAPH_POS_X 25
#define GRAPH_POS_Y 10
#define GRAPH_LEN_X 100
#define GRAPH_LEN_Y 50
#define GRAPH_DPP 10 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

#define VALUE_MIN 0
#define VALUE_MAX 1023
#define VALUE_SIZE (GRAPH_LEN_X / GRAPH_DPP) + 1
int x[VALUE_SIZE];  //x座標
int t[VALUE_SIZE];  //一定間隔に垂線を引くための配列(垂線の有無)
graph_line grline = graph_line(
                      LOOPTIME_GRAPH,
                      GRAPH_POS_X,
                      GRAPH_POS_Y,
                      GRAPH_LEN_X,
                      GRAPH_LEN_Y,
                      GRAPH_DPP,
                      VALUE_MIN,
                      VALUE_MAX,
                      VALUE_SIZE,
                      &x[0],
                      &t[0]
                    );

//折れ線グラフのグラフの設定
//頂点の数が可変なので外部で配列を用意している
#define GL_1 0
#define GL_2 1
#define GL_3 2
int v1[VALUE_SIZE]; //頂点の値
int p1[VALUE_SIZE]; //グラフでプロットするy座標
int v2[VALUE_SIZE]; //頂点の値
int p2[VALUE_SIZE]; //グラフでプロットするy座標
int v3[VALUE_SIZE]; //頂点の値
int p3[VALUE_SIZE]; //グラフでプロットするy座標

void setup() {
  Nefry.enableSW();

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  //表示するグラフの設定
  grline.setGraph(GL_1, &v1[0], &p1[0], VERTEX_NONE, DISP_MAX);
  grline.setGraph(GL_2, &v2[0], &p2[0], VERTEX_CIR, NOTDISP_MAX);
  grline.setGraph(GL_3, &v3[0], &p3[0], VERTEX_SQU, NOTDISP_MAX);

  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));
}

void dispGraphLine_update() {
  //領域の描画
  grline.dispArea();
  grline.updateGraph();
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(35, 0, String(sampleData[0]));
  NefryDisplay.drawString(70, 0, String(sampleData[1]));
  NefryDisplay.drawString(105, 0, String(sampleData[2]));
  dispGraphLine_update();
  NefryDisplay.display();
}


void loop() {
  if (Nefry.readSW()) {
    isStopGraph = !isStopGraph;
  }


  interval<LOOPTIME_GRAPH>::run([] {
    if (!isStopGraph) {
      sampleData[0] = random(VALUE_MIN + 300, VALUE_MAX + 1 - 300);
      sampleData[1] = (VALUE_MAX / 2) * (1 + sin(deg / (180 / PI)));
      sampleData[2] = (VALUE_MAX / 2) * (1 + cos(deg / (180 / PI)));
      deg += 10;

      grline.addGraphData(GL_1, (int)sampleData[0]);
      grline.addGraphData(GL_2, (int)sampleData[1]);
      grline.addGraphData(GL_3, (int)sampleData[2]);
      grline.updateGraphTime();
    }

    DispNefryDisplay();
  });
}
