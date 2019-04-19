#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
void setting() {
  Nefry.disableDisplayStatus();
  Nefry.disableWifi();
}
NefrySetting nefrySetting(setting);

#define PIN_DIGITAL_SW D8
#define PIN_ANALOG_SW A3

//ループ周期(us)
#include <interval.h>
#define LOOPTIME_ANALOG 30000

void setup() {
  NefryDisplay.begin();
  NefryDisplay.setAutoScrollFlg(true);//自動スクロールを有効

  NefryDisplay.clear();
  NefryDisplay.display();

  dispGraph_init();
  NefryDisplay.autoScrollFunc(DispNefryDisplay);

  // put your setup code here, to run once:
  pinMode(PIN_DIGITAL_SW, INPUT);
  pinMode(PIN_ANALOG_SW, INPUT);

}


#define GRAPH_MAX_X 129 //128 + 1
#define GRAPH_MAX_Y 65  //64 + 1
#define VALUE_MAX 1023
int v[GRAPH_MAX_X];
int idx;
void dispGraph_init() {
  idx = 0;
  for (int i = 0; i < GRAPH_MAX_X; i++) {
    v[i] = GRAPH_MAX_Y;
  }
}

void dispGraph_set(int _v) {
  //前にずらす
  for (int i = 0; i < GRAPH_MAX_X - 1; i++) {
    v[i] = v[i + 1];
  }
  //最後尾に追加する
  v[idx] = map(_v, 0, VALUE_MAX, GRAPH_MAX_Y, 0);
  idx++;
  if (idx >= GRAPH_MAX_X) idx = GRAPH_MAX_X - 1;
}

void dispGraph_update() {
  for (int i = 0; i < GRAPH_MAX_X; i++) {
    NefryDisplay.drawLine(i, v[i], i + 1, v[i + 1]);
  }
}

void DispNefryDisplay() {
  NefryDisplay.clear();
  //取得したデータをディスプレイに表示
  NefryDisplay.setFont(ArialMT_Plain_16);
  NefryDisplay.drawString(0, 0, String(digitalRead(PIN_DIGITAL_SW)));
  NefryDisplay.drawString(30, 0, String(analogRead(PIN_ANALOG_SW)));
  NefryDisplay.drawString(80, 0, String(v[idx]));
  dispGraph_update();

  NefryDisplay.display();
  Nefry.ndelay(10);
}


void loop() {
  interval<LOOPTIME_ANALOG>::run([] {
    dispGraph_set(analogRead(PIN_ANALOG_SW));
  });
}
