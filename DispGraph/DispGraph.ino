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

//サンプルデータ
long sampleData;

void setup() {
  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  dispGraph_init();

  //サンプルデータ生成用乱数シード
  randomSeed(analogRead(0));
}


#define LOOPTIME_GRAPH 100000
#define GRAPH_POS_X 25
#define GRAPH_POS_Y 10
#define GRAPH_LEN_X 100
#define GRAPH_LEN_Y 50
#define GRAPH_DPP 5 //点をプロットする間隔(1なら1ドットにつき1点、2なら2ドットにつき1点)

#define VALUE_MIN 0
#define VALUE_MAX 1023
#define VALUE_SIZE (GRAPH_LEN_X / GRAPH_DPP) + 1
int p[VALUE_SIZE][2];//グラフでプロットするx座標,データ(グラフ用に変換前)
int vMax[2];  //最大値,グラフでプロットする位置

void dispGraph_init() {
  for (int i = 0; i < VALUE_SIZE; i++) {
    p[i][0] = GRAPH_POS_X + i * GRAPH_DPP;
    p[i][1] = 0;
  }
}

void dispGraph_set(int _v) {
  vMax[0] = _v;  //最大値に仮設定
  //前にずらす
  for (int i = 0; i < VALUE_SIZE; i++) {
    p[i][1] = p[i + 1][1];
    if(p[i + 1][1] > vMax[0]) vMax[0] = p[i + 1][1];
  }
  //最後尾に追加する
  p[VALUE_SIZE - 1][1] = _v;
  vMax[1] = map(vMax[0], VALUE_MIN, VALUE_MAX, GRAPH_LEN_Y + GRAPH_POS_Y, GRAPH_POS_Y);
}

void dispGraph_update() {
  //補助線
  NefryDisplay.drawHorizontalLine(GRAPH_POS_X, GRAPH_POS_Y, GRAPH_LEN_X);
  NefryDisplay.drawHorizontalLine(GRAPH_POS_X, GRAPH_POS_Y + GRAPH_LEN_Y, GRAPH_LEN_X);
  NefryDisplay.drawHorizontalLine(GRAPH_POS_X, vMax[1], GRAPH_LEN_X);
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(GRAPH_POS_X - 20, GRAPH_POS_Y - 8, String(VALUE_MAX));
  NefryDisplay.drawString(GRAPH_POS_X - 20, GRAPH_POS_Y + GRAPH_LEN_Y - 8, String(VALUE_MIN));
  NefryDisplay.drawString(GRAPH_POS_X - 20, vMax[1] - 8, String(vMax[0]));

  int p1;
  int p2;
  for (int i = 0; i < VALUE_SIZE - 1; i++) {
    //グラフ向けの値に変換
    p1 = map(p[i][1], VALUE_MIN, VALUE_MAX, GRAPH_LEN_Y + GRAPH_POS_Y, GRAPH_POS_Y);
    p2 = map(p[i+1][1], VALUE_MIN, VALUE_MAX, GRAPH_LEN_Y + GRAPH_POS_Y, GRAPH_POS_Y);
    
    NefryDisplay.drawLine(p[i][0], p1, p[i + 1][0], p2);  //各座標を線で結ぶ
    //各座標をプロットする
    NefryDisplay.fillCircle(p[i][0], p1, 2); //頂点を丸くする
    //NefryDisplay.fillRect(p[i][0] - 2, p1 - 2, 4, 4); //頂点を四角くする
  }
  //最後の点をプロット
  p1 = map(p[VALUE_SIZE - 1][1], VALUE_MIN, VALUE_MAX, GRAPH_LEN_Y + GRAPH_POS_Y, GRAPH_POS_Y);
  NefryDisplay.fillCircle(p[VALUE_SIZE - 1][0], p1, 2); //頂点を丸くする
  //NefryDisplay.fillRect(p[VALUE_SIZE - 1][0] - 2, p1 - 2, 4, 4); //頂点を四角くする

}

void DispNefryDisplay() {
  NefryDisplay.clear();
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_10);
  NefryDisplay.drawString(105, 0, String(sampleData));
  dispGraph_update();
  NefryDisplay.display();
}


void loop() {
  interval<LOOPTIME_GRAPH>::run([] {
    sampleData = random(VALUE_MIN, VALUE_MAX + 1 - 300);
    dispGraph_set((int)sampleData);
    DispNefryDisplay();
  });
}
