/*
 * This sample sketch is LCD ILI9341 display test of M5Stack.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2018 Mgo-tec. All rights reserved.
 * 
 * Modify Display.cpp of M5Stack library.
 * M5Stack library - MIT License
 * Copyright (c) 2017 M5Stack
 * 
 * https://opensource.org/licenses/mit-license.php
 */
#include <SPI.h>
#include "soc/spi_reg.h"
  
#define SPI_NUM 0x3 //VSPI=0x3, HSPI=0x2
  
const int _sck = 18; // SPI clock pin
const int _miso = -1; // MISO(master input slave output) don't using
const int _mosi = 23; // MOSI(master output slave input) pin
const int _cs = 14; // Chip Select pin
const int _dc = 27; // Data/Command pin
const int _rst = 33; // Reset pin
const uint8_t _LCD_LEDpin = 32;
  
//***********セットアップ****************************
void setup() {
  Serial.begin(115200);
  ILI9341_Init();
  Display_Clear(0, 0, 319, 239);
  Brightness(255); //LCD LED Full brightness
  delay(2000);
}
//***********メインループ****************************
void loop() {
  int i, j;
  //red (0-31), green (0-63), blue (0-31)
  uint8_t red_max = 31, green_max = 63, blue_max = 31;
  Draw_Pixel_65k_3Color(0, 0, red_max, green_max, blue_max);
  Draw_Pixel_65k_3Color(319, 0, red_max, green_max, blue_max);
  Draw_Pixel_65k_3Color(0, 239, red_max, green_max, blue_max);
  Draw_Pixel_65k_3Color(319, 239, red_max, green_max, blue_max);
  delay(2000);
  
  Draw_Horizontal_Line(0, 319, 0, red_max, green_max, blue_max);
  Draw_Horizontal_Line(0, 319, 239, red_max, green_max, blue_max);
  Draw_Vertical_Line(0, 0, 239, red_max, green_max, blue_max);
  Draw_Vertical_Line(319, 0, 239, red_max, green_max, blue_max);
  delay(2000);
  
  Draw_Line(0, 0, 319, 239, red_max, 0, 0);
  Draw_Line(319, 0, 0, 239, 0, green_max, 0);
  Draw_Line(160, 0, 160, 239, 0, 0, blue_max);
  Draw_Line(0, 120, 319, 120, red_max, green_max, 0);
  delay(2000);
  
  for(i=0; i<120; i=i+5){
    Draw_Rectangle_Line(40+i, i, 279-i, 239-i, red_max, green_max, blue_max);
  }
  delay(2000);
  
  uint8_t Width = 50;
  Draw_Rectangle_Fill(0, 0, 319, 239, 0, 0, blue_max);
  Draw_Rectangle_Fill(10, 10, 10+Width, 10+Width, red_max, 0, 0);
  Draw_Rectangle_Fill(309-Width, 10, 309, 10+Width, 0, green_max, 0);
  Draw_Rectangle_Fill(10, 229-Width, 10+Width, 229, red_max, green_max, 0);
  Draw_Rectangle_Fill(309-Width, 229-Width, 309, 229, 0, green_max, blue_max);
  delay(2000);
  
  Display_Clear(0, 0, 319, 239);
  Draw_Rectangle_Line(40, 0, 279, 239, 0, 0, blue_max);
  Draw_Line(40, 0, 279, 239, 0, 0, blue_max);
  Draw_Line(40, 239, 279, 0, 0, 0, blue_max);
  Draw_Circle_Line(159, 119, 119, red_max, green_max, blue_max);
  Draw_Circle_Line(25, 25, 25, red_max, 0, 0);
  Draw_Circle_Line(294, 25, 25, 0, green_max, 0);
  Draw_Circle_Line(25, 214, 25, red_max, green_max, 0);
  Draw_Circle_Line(294, 214, 25, red_max, 0, blue_max);
  for(i=0; i<120; i=i+5){
    Draw_Circle_Line(159, 119, 119-i, red_max, green_max, blue_max);
  }
  delay(2000);
  
  Display_Clear(0, 0, 319, 239);
  Draw_Rectangle_Fill(0, 0, 319, 239, 0, 0, 20);
  Draw_Circle_Fill(159, 119, 119, red_max, 0, blue_max);
  Draw_Circle_Fill(25, 25, 25, red_max, 0, 0);
  Draw_Circle_Fill(294, 25, 25, 0, green_max, 0);
  Draw_Circle_Fill(25, 214, 25, red_max, green_max, 0);
  Draw_Circle_Fill(294, 214, 25, 0, green_max, blue_max);
  Draw_Circle_Fill(159, 119, 51, 0, 0, blue_max);
  delay(2000);
  
  Display_Clear(0, 0, 319, 239);
  for(i=0; i<240; i=i+10){
    Draw_Line(0, i, i, 239, 10, green_max, 20);
    Draw_Line(80+i, 0, 319, i, 0, green_max, 31);
  }
  delay(2000);
  
  Display_Clear(0, 0, 319, 239);
  j = 31;
  for(i=0; i<32; i++){
    Draw_Rectangle_Fill(i*10, 0, i*10+9, 59, i, 0, 0);
    Draw_Rectangle_Fill(i*10, 60, i*10+9, 119, 0, i*2, 0);
    Draw_Rectangle_Fill(i*10, 120, i*10+9, 179, 0, 0, i);
    Draw_Rectangle_Fill(i*10, 180, i*10+9, 239, j, j*2, j);
    j--;
  }
  delay(10000);
  Display_Clear(0, 0, 319, 239);
}
//****** LCD ILI9341 ディスプレイ初期化 ***********
void ILI9341_Init(){
  Brightness(0);
  
  pinMode(_rst, OUTPUT); //Set RESET pin
  pinMode(_dc, OUTPUT); //Set Data/Command pin
  
  SPI.begin(_sck, _miso, _mosi, _cs); //VSPI setting
  
  SPI.setBitOrder(MSBFIRST);
  //ILI9341 のSPI Clock Cycle Time (Write) Minimun 100ns=10MHz
  //SPI.setFrequency(10000000);
  SPI.setFrequency(27000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.setHwCs(true); //Set Hardware CS pin
  
  //Hardware Reset------------
  digitalWrite(_rst, HIGH);
  delay(5);
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(121);
  
  digitalWrite(_dc, HIGH);
  
  CommandWrite(0x38); //Idle mode OFF
  CommandWrite(0x3A); //COLMOD: Pixel Format Set
    DataWrite(0b01010101); //RGB 16 bits / pixel, MCU 16 bits / pixel
  CommandWrite(0x36); //MADCTL: Memory Access Control
    DataWrite(0b00001000); //D3: BGR(RGB-BGR Order control bit )="1"
  CommandWrite(0x11); //Sleep OUT
  delay(10);
  
  CommandWrite(0x29); //Display ON
  
  Display_Clear(0, 0, 319, 239);
  Brightness(100);
}
//********* 4wire SPI Data / Command write************
void CommandWrite(uint8_t b){
  digitalWrite(_dc, LOW);
  SPI.write(b);
  digitalWrite(_dc, HIGH);
}
  
void DataWrite(uint8_t b){
  SPI.write(b);
}
  
void DataWrite16(uint16_t b){
  SPI.write16(b);
}
  
void DataWrite32(uint32_t b){
  SPI.write32(b);
}
//******** Set Column and Page Address ( X Y range setting )***********
void XY_Range(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
  uint32_t X = (uint32_t)x0<<16 | x1;
  uint32_t Y = (uint32_t)y0<<16 | y1;
    
  CommandWrite( 0x2A ); //Set Column Address
    DataWrite32(X);
  CommandWrite( 0x2B ); //Set Page Address
    DataWrite32(Y);
}
//********* Display All Black Clear ******************************
void Display_Clear(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
  uint16_t Width_x = x1 - x0 + 1;
  uint16_t Width_y = y1 - y0 + 1;
  uint32_t Total = Width_x * Width_y ;
  
  Block_SPI_Fast_Write(x0, y0, x1, y1, 0, 0, 0, Total);
}
//********* Display Color Pixel Block Fast Write *****************
void spiWriteBlock(uint16_t color, uint32_t repeat) {
  uint16_t color16 = (color >> 8) | (color << 8);
  uint32_t color32 = color16 | color16 << 16;
  
  if (repeat > 15) {
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(SPI_NUM), SPI_USR_MOSI_DBITLEN, 255,
                      SPI_USR_MOSI_DBITLEN_S);
  
    while (repeat > 15) {
      while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)
        ;
      for (uint32_t i = 0; i < 16; i++)
        WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), color32);
      SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
      repeat -= 16;
    }
    while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)
      ;
  }
  
  if (repeat) {
    repeat = (repeat << 4) - 1;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(SPI_NUM), SPI_USR_MOSI_DBITLEN, repeat,
                      SPI_USR_MOSI_DBITLEN_S);
    for (uint32_t i = 0; i < 16; i++)
      WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), color32);
    SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)
      ;
  }
}
//*********** LCD ILE9341 Block Pixel SPI Fast Write *****************
void Block_SPI_Fast_Write(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t red, uint8_t green, uint8_t blue, uint32_t repeat){
  uint16_t ColorDot = (red << 11) | (green << 5) | blue;
  XY_Range(x0, y0, x1, y1);
  CommandWrite( 0x2C ); //LCD RAM write
  spiWriteBlock(ColorDot, repeat);
}
//*********** 65k Color Pixel (Dot) Write ****************************
void Draw_Pixel_65k_DotColor(uint16_t x0, uint16_t y0, uint16_t DotColor){
  XY_Range(x0, y0, x0, y0);
  CommandWrite( 0x2C ); //RAM write
  DataWrite16( DotColor );
}
//*********** 65k Pixel RGB color Write ****************************
void Draw_Pixel_65k_3Color(uint16_t x0, uint16_t y0, uint8_t red, uint8_t green, uint8_t blue){
  //red (0-31), green (0-63), blue (0-31)
  XY_Range(x0, y0, x0, y0);
  
  uint16_t Dot = ((uint16_t)red << 11) | ((uint16_t)green << 5) | (uint16_t)blue;
  
  CommandWrite( 0x2C ); //RAM write
  DataWrite16( Dot );
}
//***************************************
void Draw_Rectangle_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t red, uint8_t green, uint8_t blue){
  Draw_Horizontal_Line(x0, x1, y0, red, green, blue);
  Draw_Horizontal_Line(x0, x1, y1, red, green, blue);
  Draw_Vertical_Line(x0, y0, y1, red, green, blue);
  Draw_Vertical_Line(x1, y0, y1, red, green, blue);
}
//***************************************
void Draw_Horizontal_Line(int16_t x0, int16_t x1, int16_t y0, uint8_t red, uint8_t green, uint8_t blue){
  if(x0 > 319) x0 = 319;
  if(x1 > 319) x1 = 319;
  if(y0 > 239) y0 = 239;
  if(x1 < x0){
    uint16_t dummy = x1;
    x1 = x0;
    x0 = dummy;
  }
  
  uint32_t Width_x = x1 - x0 + 1;
  Block_SPI_Fast_Write(x0, y0, x1, y0, red, green, blue, Width_x);
}
//***************************************
void Draw_Vertical_Line(int16_t x0, int16_t y0, int16_t y1, uint8_t red, uint8_t green, uint8_t blue){
  if(x0 > 319) x0 = 319;
  if(y0 > 239) y0 = 239;
  if(y1 > 239) y1 = 239;
  if(y1 < y0){
    uint16_t dummy = y1;
    y1 = y0;
    y0 = dummy;
  }
  
  uint16_t Width_y = y1 - y0 + 1;
  Block_SPI_Fast_Write(x0, y0, x0, y1, red, green, blue, Width_y);
}
//***************************************
void Draw_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t red, uint8_t green, uint8_t blue){
  if(x0 > 319) x0 = 319;
  if(x1 > 319) x1 = 319;
  if(y0 > 239) y0 = 239;
  if(y1 > 239) y1 = 239;
  if(x0 == x1 && y0 == y1) return;
  
  int i;
 
  int16_t Y = 0, X = 0;
  int16_t length_x = x1 - x0;
  int16_t length_y = y1 - y0;
  
  uint16_t Dot = (red << 11) | (green << 5) | blue;
  
  if(abs(length_x) > abs(length_y)){
    float degY = ((float)length_y) / ((float)length_x);
    if(x0 < x1){
      for(i=x0; i<(x1+1); i++){
        Y = y0 + round((i-x0) * degY);
        Draw_Pixel_65k_DotColor(i, Y, Dot);
      }
    }else{
      for(i=x0; i>=x1; i--){
        Y = y0 + round((i-x0) * degY);
        Draw_Pixel_65k_DotColor(i, Y, Dot);
      }
    }
  }else{
    float degX = ((float)length_x) / ((float)length_y);
  
    if(y0 < y1){
      for(i=y0; i<(y1+1); i++){
        X = x0 + round((i-y0) * degX);
        Draw_Pixel_65k_DotColor(X, i, Dot);
      }
    }else{
      for(i=y0; i>=y1; i--){
        X = x0 + round((i-y0) * degX);
        Draw_Pixel_65k_DotColor(X, i, Dot);
      }
    }
  }
}
//***************************************
void Draw_Rectangle_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t red, uint8_t green, uint8_t blue){
  uint16_t Width_x = x1 - x0 + 1;
  uint16_t Width_y = y1 - y0 + 1;
  uint32_t Total = Width_x * Width_y ;
  Block_SPI_Fast_Write(x0, y0, x1, y1, red, green, blue, Total);
}
//***************************************
void Draw_Circle_Line(uint16_t x0, uint16_t y0, uint16_t r, uint8_t red, uint8_t green, uint8_t blue){
  uint16_t x1, y1;
  float i;
  float deg = 1.0;
  if( r > 50 ) deg = 0.5;
  if( r > 110) deg = 0.25;
  
  uint16_t Dot = ((uint16_t)red << 11) | ((uint16_t)green << 5) | (uint16_t)blue;
  
  for(i=0; i<360; i=i+deg){
    x1 = round( (float)(x0 + (r * cos(radians(i)))) );
    y1 = round( (float)(y0 + (r * sin(radians(i)))) );
    Draw_Pixel_65k_DotColor(x1, y1, Dot);
  }
}
//***************************************
void Draw_Circle_Fill(uint16_t x0, uint16_t y0, uint16_t r, uint8_t red, uint8_t green, uint8_t blue){
  //red (0-31), green (0-63), blue (0-31)
  uint16_t x1, y1;
  float i;
  float deg = 1.0;
  //半径が大きくなると、角度の刻み方を細かくしないと、完全に塗りつぶせないので注意。
  if( r > 50 ) deg = 0.5;
  if( r > 110) deg = 0.25;
  
  for( i = 0; i < 360; i = i + deg ){
    x1 = round( (float)(x0 + (r * cos(radians(i)))) );
    y1 = round( (float)(y0 + (r * sin(radians(i)))) );
    Draw_Vertical_Line(x1, y0, y1, red, green, blue);
  }
}
//********* LCD Display LED Brightness **************
void Brightness(uint8_t brightness){
  uint8_t ledc_ch = 0;
  uint32_t valueMax = 255;
  uint32_t duty = (8191 / valueMax) * min((uint32_t)brightness, valueMax);
  ledcSetup(ledc_ch, 5000, 13);
  ledcAttachPin(_LCD_LEDpin, 0);
  ledcWrite(ledc_ch, duty);
}
