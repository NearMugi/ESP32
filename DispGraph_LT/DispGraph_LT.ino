//LT用に以下のプロジェクトを合体する。
//DispGraph.ino
//LissajousFigure.ino
//DispWatch.ino

#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
}
NefrySetting nefrySetting(setting);

//データ
//加速度センサー(MPU6050)
#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;
//MPU6050の初期化時に使用するオフセット
int CalOfs[4] = { -263, -36, -13, 1149}; //Gyro x,y,z, Accel z
int mpu6050_WorldAccel[3];       //[x,y,z]
//Groveの音センサー
float dataSound;
float RC_a = 0.95;  //ローパスフィルタの係数
//sin,cos
float dataSin;
float dataCos;
float deg = 0;


// Grove SoundSensor
#define PIN_ADC  A1
//切り替えボタン
#define PIN_SW D8

//date
#include <time.h>
#define JST     3600*9

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
  LISSAJOUS,
  WATCH,
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


//++++++++++++++++++++++++++++++++++++++++++++
//折れ線グラフ
//++++++++++++++++++++++++++++++++++++++++++++
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

//グラフの設定
//頂点の数が可変なので外部で配列を用意している
int v1[LINE_PLOT_SIZE]; //頂点の値
int v2[LINE_PLOT_SIZE]; //頂点の値
int v3[LINE_PLOT_SIZE]; //頂点の値

//グラフの初期化
void dispGraphLine_init() {
  grline.initGraphTime();
  grline.setGraph(GR_1, &v1[0], VERTEX_CIR, DISP_MAX);
  grline.setGraph(GR_2, &v2[0], VERTEX_NONE, NOTDISP_MAX);
  grline.setGraph(GR_3, &v3[0], VERTEX_NONE, NOTDISP_MAX);
}

//グラフの描画
void dispGraphLine_update() {
  grline.dispArea();
  grline.updateGraph();
}

//++++++++++++++++++++++++++++++++++++++++++++
//棒グラフ
//++++++++++++++++++++++++++++++++++++++++++++
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


//グラフの設定
//保存するデータ数が可変なので外部で配列を用意している
//縦方向と横方向は同じデータを使う
int vb1[BAR_PLOT_SIZE];
int vb2[BAR_PLOT_SIZE];
int vb3[BAR_PLOT_SIZE];

//グラフの初期化
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

//++++++++++++++++++++++++++++++++++++++++++++
//円グラフ
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphCircle.h"
graph_circle grcir = graph_circle();

//グラフの初期化
void dispGraphCircle_init() {
  //グラフ番号・頂点座標(x,y)・半径(r)・対象の値(最小値,最大値)・塗りつぶし有り無し
  grcir.setGraph(GR_1, 20, 35, 20, 0, 1023, false);
  grcir.setGraph(GR_2, 62, 35, 20, 0, 1023, true);
  grcir.setGraph(GR_3, 104, 35, 20, 0, 1023, true);
}
//グラフの描画
void dispGraphCircle_update() {
  grcir.dispArea();
  grcir.updateGraph();
}

//++++++++++++++++++++++++++++++++++++++++++++
//リサージュ図形の設定
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphLissajousFigure.h"
//static変数を定義
int graph_lissajous::valueSIZE;

//n秒分プロットする
#define PLOTTIME_US 2000000
#define LISSAJOUS_PLOT_SIZE (PLOTTIME_US / LOOPTIME_GRAPH)

graph_lissajous grlissajous[2];

//頂点の数が可変なので外部で配列を用意している
float p0[LISSAJOUS_PLOT_SIZE][2]; //プロットする値(変換前)
float p1[LISSAJOUS_PLOT_SIZE][2]; //プロットする値(変換前)

//初期化
void dispGraphLissajous_init() {
  //グラフ領域
  //左上の座標(x,y)　※中心座標ではない
  //x,y方向のドット数
  //値の最小値,最大値
  //プロット数
  grlissajous[0] = graph_lissajous( 2, 2, 60, 60, -8192, 8192, LISSAJOUS_PLOT_SIZE);
  grlissajous[1] = graph_lissajous( 64, 2, 60, 60, -8192, 8192, LISSAJOUS_PLOT_SIZE);
  grlissajous[0].setGraph(&p0[0][0]);
  grlissajous[1].setGraph(&p1[0][0]);
}

//リサージュ図形の描画
void dispGraphLissajous_update() {
  grlissajous[0].dispArea();
  grlissajous[0].updateGraph();

  grlissajous[1].dispArea();
  grlissajous[1].updateGraph();
}

//++++++++++++++++++++++++++++++++++++++++++++
//時計
//++++++++++++++++++++++++++++++++++++++++++++
#include "dispGraphCircle_Watch.h"
graph_circle_watch grwatch = graph_circle_watch();

uint8_t Hour = 0;
uint8_t Min = 0;
uint8_t Sec = 0;

void dispWatch_init() {
  //グラフ番号・頂点座標(x,y)・半径(r)・対象の値(最小値,最大値)・塗りつぶし有り無し
  grwatch.setGraph(CL_HOUR, CL_X, CL_Y, 20, 0, 60, false);
  grwatch.setGraph(CL_MIN, CL_X, CL_Y, 25, 0, 60, false);
  grwatch.setGraph(CL_SEC, CL_X, CL_Y, 10, 0, 60, true);
}

void dispWatch_update() {
  //外枠
  NefryDisplay.drawCircle(CL_X, CL_Y, CL_R);

  //時計の針の描画
  grwatch.updateGraph();

  NefryDisplay.setFont(ArialMT_Plain_24);
  String _tmpH = String(Hour);
  if (Hour < 10) _tmpH = '0' + String(Hour);
  String _tmpM = String(Min);
  if (Min < 10) _tmpM = '0' + String(Min);
  String _tmpS = String(Sec);
  if (Sec < 10) _tmpS = '0' + String(Sec);

  NefryDisplay.drawString(70, 0, _tmpH);
  NefryDisplay.drawString(70, 21, _tmpM);
  NefryDisplay.drawString(70, 42, _tmpS);
}

//音データの取得　※RCフィルタ適用
long getSoundAnalogValue(float y_old) {
  int x = analogRead(PIN_ADC);
  float y = RC_a * y_old + (1 - RC_a) * x;
  return y;
}


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

  mpu_main.init(false, CalOfs);

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
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
      case LISSAJOUS:
        dispGraphLissajous_init();
        break;
      case WATCH:
        dispWatch_init();
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

      //サンプルデータの生成
      mpu_main.updateValue();
      mpu_main.Get_WorldAccel(mpu6050_WorldAccel);

      dataSound = getSoundAnalogValue(dataSound);
      dataSin = (VALUE_LINE_MAX / 2) * (1 + sin(deg / (180 / PI)));
      dataCos = (VALUE_LINE_MAX / 2) * (1 + cos(deg / (180 / PI)));
      deg += 10;

      switch (dispType) {
        case LINE:
          grline.addGraphData(GR_1, (int)dataSound);
          grline.addGraphData(GR_2, (int)dataSin);
          grline.addGraphData(GR_3, (int)dataCos);
          grline.updateGraphTime();
          break;
        case BAR_V:
          grbar[0].addGraphData(GR_1, (int)dataSound);
          grbar[0].addGraphData(GR_2, (int)dataSin);
          grbar[0].addGraphData(GR_3, (int)dataCos);
          grbar[0].updateGraphTime();
          break;
        case BAR_S:
          grbar[1].addGraphData(GR_1, (int)dataSound);
          grbar[1].addGraphData(GR_2, (int)dataSin);
          grbar[1].addGraphData(GR_3, (int)dataCos);
          grbar[1].updateGraphTime();
          break;
        case CIRCLE:
          grcir.addGraphData(GR_1, (int)dataSound);
          grcir.addGraphData(GR_2, (int)dataSin);
          grcir.addGraphData(GR_3, (int)dataCos);
          break;
        case LISSAJOUS:
          grlissajous[0].addGraphData(mpu6050_WorldAccel[0], mpu6050_WorldAccel[1]);
          grlissajous[1].addGraphData(mpu6050_WorldAccel[0], mpu6050_WorldAccel[2]);
          break;
        case WATCH:
          //日付を取得する
          time_t  t = time(NULL);
          struct tm *tm;
          tm = localtime(&t);
          Hour = tm->tm_hour;
          Min = tm->tm_min;
          Sec = tm->tm_sec;
          //24h -> 12h表記にする
          uint8_t _tmpHour = Hour % 12;
          //分に合わせて時間の針を進める
          _tmpHour = _tmpHour * 5 + Min / 12;

          grwatch.addGraphData(CL_HOUR, (int)_tmpHour);
          grwatch.addGraphData(CL_MIN, (int)Min);
          grwatch.addGraphData(CL_SEC, (int)Sec);
          break;
      }
    }

    //描画する
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
      case LISSAJOUS:
        dispGraphLissajous_update();
        break;
      case WATCH:
        dispWatch_update();
        break;
      default:
        NefryDisplay.setFont(ArialMT_Plain_16);
        NefryDisplay.drawString(0, 0, "Select GraphType...");
        break;
    }
    NefryDisplay.display();


  });
}
