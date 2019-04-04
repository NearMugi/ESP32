import processing.serial.*;
Serial myPort;
String PORT = "COM6";
int lf = 10;      // ASCII linefeed 

//参考
//https://garchiving.com/real-time-graph-by-proccesing/
graphMonitor testGraph;



float ave_unit;
float ave_sec;
float maxAnalogValue;

void setup() {
  myPort = new Serial(this, PORT, 115200);
  myPort.bufferUntil(lf); 
  println("--- SerialPort [" + PORT + "] Connect ---");
  
  size(1200, 600, P3D);
  frameRate(100);
  smooth();
  testGraph = new graphMonitor("SoundSensor", 100, 50, 1000, 400);
  
}

void draw() {
  background(0);
  testGraph.graphDraw(ave_unit, ave_sec,maxAnalogValue);
  
  pushMatrix();
  translate(20, 500);
  textSize(25);
  fill(255);
  textAlign(LEFT, BOTTOM);
  text("ave_unit  :" + ave_unit, 0, 0);
  text("ave_sec   :" + ave_sec, 0, 25);
  text("MAX VALUE :" + maxAnalogValue, 0, 50);
  popMatrix();
  
}

void serialEvent(Serial p){
  String myString = p.readStringUntil('\n');
  
  myString = trim(myString);
  
  float value[] = float(split(myString, ','));
  
  if (value.length == 3) {
    ave_unit = value[0];
    ave_sec = value[1];
    maxAnalogValue = value[2];
  }  
//  println(ave_unit + " , " + ave_sec);
}

class graphMonitor {
    String TITLE;
    int X_POSITION, Y_POSITION;
    int X_LENGTH, Y_LENGTH;
    float [] y1, y2, y3;
    float maxRange;
    graphMonitor(String _TITLE, int _X_POSITION, int _Y_POSITION, int _X_LENGTH, int _Y_LENGTH) {
      TITLE = _TITLE;
      X_POSITION = _X_POSITION;
      Y_POSITION = _Y_POSITION;
      X_LENGTH   = _X_LENGTH;
      Y_LENGTH   = _Y_LENGTH;
      y1 = new float[X_LENGTH];
      y2 = new float[X_LENGTH];
      y3 = new float[X_LENGTH];
      for (int i = 0; i < X_LENGTH; i++) {
        y1[i] = 0;
        y2[i] = 0;
        y3[i] = 0;
      }
      
      maxRange = 1023;
    }

    void graphDraw(float _y1, float _y2) {
      graphDraw(_y1, _y2, 0);
    }
    
    void graphDraw(float _y1, float _y2, float _y3) {
      y1[X_LENGTH - 1] = _y1;
      y2[X_LENGTH - 1] = _y2;
      y3[X_LENGTH - 1] = _y3;
      for (int i = 0; i < X_LENGTH - 1; i++) {
        y1[i] = y1[i + 1];
        y2[i] = y2[i + 1];
        y3[i] = y3[i + 1];
      }
      
      pushMatrix();

      translate(X_POSITION, Y_POSITION);
      fill(25);
      stroke(130);
      strokeWeight(1);
      rect(0, 0, X_LENGTH, Y_LENGTH);
      line(0, Y_LENGTH *3/ 4, X_LENGTH, Y_LENGTH *3/ 4);
      line(0, Y_LENGTH *1/ 2, X_LENGTH, Y_LENGTH *1/ 2);
      line(0, Y_LENGTH *1/ 4, X_LENGTH, Y_LENGTH *1/ 4);
      
      
      //title,Value Y
      textSize(25);
      fill(255);
      textAlign(LEFT, BOTTOM);
      text(TITLE, 20, -5);
      textSize(22);
      textAlign(RIGHT);
      text(nf(maxRange, 0, 1), -5, 18);
      text(nf(maxRange*3/4, 0, 1), -5, Y_LENGTH * 1/4);
      text(nf(maxRange*1/2, 0, 1), -5, Y_LENGTH * 1/2);
      text(nf(maxRange*1/4, 0, 1), -5, Y_LENGTH * 3/4);
      text(0, -5, Y_LENGTH);
      
      //graph
      translate(0, Y_LENGTH);
      scale(1, -1);
      strokeWeight(1);
      for (int i = 0; i < X_LENGTH - 1; i++) {
        stroke(255, 0, 0);
        line(i, y1[i] * Y_LENGTH  / maxRange, i + 1, y1[i + 1] * Y_LENGTH  / maxRange);
        stroke(0, 0, 255);
        line(i, y2[i] * Y_LENGTH  / maxRange, i + 1, y2[i + 1] * Y_LENGTH  / maxRange);
        stroke(255, 255, 0);
        line(i, y3[i] * Y_LENGTH  / maxRange, i + 1, y3[i + 1] * Y_LENGTH  / maxRange);
      }
      popMatrix();
    }
}
