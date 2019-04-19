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
#define LOOPTIME_GRAPH 100000 //グラフの更新周期

//グラフ一時停止
bool isStopGraph = false;

//サンプルデータ
long sampleData[3];
float deg = 0;

//++++++++++++++++++++++++++++++++++++++++++++
//折れ線グラフ
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphLine.h"
//static変数を定義
int graph_line::valueSIZE;

#define GR_1 0
#define GR_2 1
#define GR_3 2

//グラフの表示領域
#define GRAPH_LINE_POS_X 27
#define GRAPH_LINE_POS_Y 10
#define GRAPH_LINE_LEN_X 100
#define GRAPH_LINE_LEN_Y 50
#define GRAPH_LEN_DPP 10 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

//アナログデータの最大値・最小値
#define VALUE_LINE_MIN 0
#define VALUE_LINE_MAX 4098 //esp32は分解能12bit

//プロットするデータの数
#define LINE_PLOT_SIZE (GRAPH_LINE_LEN_X / GRAPH_LEN_DPP) + 1

//データを保存する箱(配列)
//頂点の数が可変なので外部で配列を用意している
int x[LINE_PLOT_SIZE];  //x座標
int t[LINE_PLOT_SIZE];  //一定間隔に垂線を引くための配列
int v1[LINE_PLOT_SIZE]; //アナログデータ1の値
int v2[LINE_PLOT_SIZE]; //アナログデータ2の値
int v3[LINE_PLOT_SIZE]; //アナログデータ3の値

//本体
//更新周期・表示領域・アナログデータの範囲・プロット間隔・データ数・x座標と時間の箱
graph_line grline = graph_line(
                      LOOPTIME_GRAPH,
                      GRAPH_LINE_POS_X, GRAPH_LINE_POS_Y, GRAPH_LINE_LEN_X, GRAPH_LINE_LEN_Y,
                      GRAPH_LEN_DPP,
                      VALUE_LINE_MIN, VALUE_LINE_MAX,
                      LINE_PLOT_SIZE,
                      &x[0], &t[0]
                    );

//初期化
void dispGraphLine_init() {
  //時間の箱を初期化
  grline.initGraphTime();
  
  //各グラフの箱を初期化
  //  インデックス[0～2]
  //  アナログデータを保存する箱
  //  頂点のタイプ[VERTEX_NONE(頂点そのまま), VERTEX_CIR(丸), VERTEX_SQU(四角)]
  //  特定の値(最大値や平均値など)の表示有無[true:表示する,false:表示しない]
  grline.setGraph(GR_1, &v1[0], VERTEX_CIR, true);
  grline.setGraph(GR_2, &v2[0], VERTEX_NONE, false);
  grline.setGraph(GR_3, &v3[0], VERTEX_NONE, false);
}

//描画
void dispGraphLine_update() {
  grline.dispArea();
  grline.updateGraph();
}


void setup() {
  Nefry.enableSW();

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効
  
  NefryDisplay.clear();
  NefryDisplay.display();

  Nefry.setLed(0, 0, 0);
  
  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));
  
  //グラフの初期設定
  dispGraphLine_init();
}


void loop() {
  
  //グラフ更新一時停止
  if (Nefry.readSW()) {
    isStopGraph = !isStopGraph;
  }

  //データ・グラフ更新
  interval<LOOPTIME_GRAPH>::run([] {
    if (!isStopGraph) {
      
      //サンプルデータ
      sampleData[0] = random(VALUE_LINE_MIN + 1500, VALUE_LINE_MAX + 1 - 2000);
      sampleData[1] = (VALUE_LINE_MAX / 2) * (1 + sin(deg / (180 / PI)));
      sampleData[2] = (VALUE_LINE_MAX / 2) * (1 + cos(deg / (180 / PI)));
      deg += 10;
      
      
      //データを追加
      grline.addGraphData(GR_1, (int)sampleData[0]);
      grline.addGraphData(GR_2, (int)sampleData[1]);
      grline.addGraphData(GR_3, (int)sampleData[2]);

      //最大値をグラフに表示する
      int _tmpMax = 0;
      for(int i=0; i<LINE_PLOT_SIZE; i++){
        if(v1[i] > _tmpMax) _tmpMax = v1[i];
      }
      grline.setValue(GR_1, _tmpMax);
      
      //時間を更新
      grline.updateGraphTime();
    }

    //描画する
    NefryDisplay.clear();
    dispGraphLine_update();
    NefryDisplay.display();

  });
}
