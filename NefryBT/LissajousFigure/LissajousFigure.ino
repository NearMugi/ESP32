//リサージュ図形を描画
//サンプルデータに加速度を使う。
#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

#include "MPU6050_Manage.h"
MPU6050_Manage mpu_main;

//MPU6050の初期化時に使用するオフセット
//CalibrationがOFFの時に適用される
int CalOfs[4] = { -263, -36, -13, 1149}; //Gyro x,y,z, Accel z

//MPU6050から取得するデータ
int mpu6050_RealAccel[3];        //[x,y,z]
int mpu6050_WorldAccel[3];       //[x,y,z]

//切り替えボタン
#define PIN_SW D8

//ループ周期(us)
#include "interval.h"
#define LOOPTIME_GRAPH 30000

//グラフ一時停止
bool isStopGraph = false;

//サンプルデータ
float deg = 0;


#include "dispGraphLissajousFigure.h"
//static変数を定義
int graph_lissajous::valueSIZE;

//n秒分プロットする
#define PLOTTIME_US 1000000
#define PLOT_SIZE (PLOTTIME_US / LOOPTIME_GRAPH)

graph_lissajous grlissajous[2];


//頂点の数が可変なので外部で配列を用意している
float p0[PLOT_SIZE][2]; //プロットする値(変換前)
float p1[PLOT_SIZE][2]; //プロットする値(変換前)


//グラフの初期化
void dispGraphLissajous_init() {
  //グラフ領域
  //左上の座標(x,y)　※中心座標ではない
  //x,y方向のドット数
  //値の最小値,最大値
  //プロット数
//  grlissajous[0] = graph_lissajous( 2, 2, 60, 60, -1, 1, PLOT_SIZE);  //三角関数
//  grlissajous[1] = graph_lissajous( 64, 2, 60, 60, -1, 1, PLOT_SIZE); //ランダム値
  grlissajous[0] = graph_lissajous( 2, 2, 60, 60, -8192, 8192, PLOT_SIZE);
  grlissajous[1] = graph_lissajous( 64, 2, 60, 60, -8192, 8192, PLOT_SIZE);
  grlissajous[0].setGraph(&p0[0][0]);
  grlissajous[1].setGraph(&p1[0][0]);
}

//グラフの描画
void dispGraphLissajous_update() {
  grlissajous[0].dispArea();
  grlissajous[0].updateGraph();

  grlissajous[1].dispArea();
  grlissajous[1].updateGraph();
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  dispGraphLissajous_update();
  NefryDisplay.display();
}

void setup() {
  Nefry.enableSW();

  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  pinMode(PIN_SW, INPUT_PULLUP);

  dispGraphLissajous_init();

  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));

  mpu_main.init(false, CalOfs);
}

void loop() {
  //グラフ更新一時停止
  if (Nefry.readSW()) {
    isStopGraph = !isStopGraph;
  }

  //データ・グラフ更新
  interval<LOOPTIME_GRAPH>::run([] {
    mpu_main.updateValue();
    mpu_main.Get_RealAccel(mpu6050_RealAccel);
    mpu_main.Get_WorldAccel(mpu6050_WorldAccel);
    if (!isStopGraph) {
      //grlissajous[0].addGraphData(cos((deg + 30) / (180 / PI)), cos(deg / (180 / PI)));
      //grlissajous[1].addGraphData(random(-50, 50) / (float)100, random(-100, 100) / (float)100);
      grlissajous[0].addGraphData(mpu6050_WorldAccel[0],mpu6050_WorldAccel[1]);
      grlissajous[1].addGraphData(mpu6050_WorldAccel[0],mpu6050_WorldAccel[2]);
      deg += 10;
    }

    DispNefryDisplay();
  });
}
