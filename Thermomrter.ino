#include "DFRobot_GDL.h"  
#include <TouchScreen.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#if defined ARDUINO_SAM_ZERO
#define TFT_DC  7
#define TFT_CS  5
#define TFT_RST 6
/*ESP32 and ESP8266*/
#elif defined(ESP32) || defined(ESP8266)
#define TFT_DC  2
#define TFT_CS  5
#define TFT_RST 4
/*AVR series mainboard*/
#else
#define TFT_DC  2
#define TFT_CS  3
#define TFT_RST 4
#endif

/**
 * @brief Constructor  Constructors for hardware SPI communication
 * @param dc  Command pin or data line pin of SPI communication 
 * @param cs  Chip select pin for SPI communication
 * @param rst Reset pin of the screen
 * @param bl  Screen backlight pin
 */
//DFRobot_ST7789_240x240_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
DFRobot_ST7789_240x320_HW_SPI tft(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
//DFRobot_ILI9341_240x320_HW_SPI  screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
//DFRobot_ILI9488_320x480_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
/* M0 mainboard DMA transfer */
//DFRobot_ST7789_240x240_DMA_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
//DFRobot_ST7789_240x320_DMA_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
//DFRobot_ILI9341_240x320_DMA_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
//DFRobot_ILI9488_320x480_DMA_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);


/*______Assign names to colors and pressure_______*/

#define BLACK   0x0000 //Black->White
#define YELLOW    0x001F //Blue->Yellow
#define RED     0xF800 //Red->Cyan
#define PINK   0x07E0 //Green-> Pink
#define CYAN    0x07FF //Cyan -> Red
#define GREEN 0xF81F //Pink -> Green 
#define BLUE  0xFFE0 //Yellow->Blue
#define WHITE   0xFFFF //White-> Black
#define MINPRESSURE 10
#define MAXPRESSURE 1000


//#define DEBUG
//Comment this out to remove the text overlay
//#define SHOW_TEMP_TEXT

//low range of the sensor (this will be blue on the screen)
#define MINTEMP 10  // minimum temperature value can be 0 degrees Celsius

//high range of the sensor (this will be red on the screen)
#define MAXTEMP 39  // maximum temperature value can be 80 degrees Celsius


//the colors we will be using

const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

Adafruit_AMG88xx amg;
unsigned long delayTime;

#define AMG_COLS 8
#define AMG_ROWS 8

float pixels[AMG_COLS * AMG_ROWS];
float pixels2[AMG_COLS * AMG_ROWS];

#define INTERPOLATED_COLS 30  //24
#define INTERPOLATED_ROWS 30  //24


float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, 
float *dest, uint8_t dest_rows, uint8_t dest_cols);

void setup() {

  delay(500);
  Serial.begin(115200);  
  Serial.println("\n\nAMG88xx Interpolated Thermal Camera!");

  tft.begin();
    tft.setRotation(0);            //PORTRAIT
    tft.fillScreen(COLOR_RGB565_BLACK);

  colorbar();

    

  // default settings
  if (!amg.begin()) {

    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1) { delay(1); }

  }

  Serial.println("-- Thermal Camera Test --");

}



float pix_max,pos_x,pos_y;


void loop() {

  //read all the pixels
 // digitalWrite(ledPin, HIGH);

  amg.readPixels(pixels2);

  for (int i=0;i<64;i++)
    pixels[i]=(pixels2[((((int)(i/8)*8)+7)-(i%8))])+2.5;    //error +2.5c

    
#ifdef DEBUG

  Serial.print("[");
  for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){

    Serial.print(pixels[(i-1)]);
    Serial.print(", ");
    if( i%8 == 0 ) Serial.println();

  }

  Serial.println("]");
  Serial.println();

#endif



  float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];
  int32_t t = millis();
  interpolate_image(pixels, AMG_ROWS, AMG_COLS, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);

#ifdef DEBUG

  Serial.print("Interpolation took "); Serial.print(millis()-t); Serial.println(" ms");

#endif

  uint16_t boxsize = min(tft.width() / INTERPOLATED_COLS, tft.height() / INTERPOLATED_COLS);
  drawpixels(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, boxsize, boxsize, false);


  tft.setTextColor(COLOR_RGB565_RED);
  tft.setCursor(0,250);  
  tft.setTextSize(3);
  tft.print("MAX:");
  tft.fillRect(70, 250, 120,30,COLOR_RGB565_BLACK );
  tft.print(pix_max);
  tft.print(" C");
  tft.drawCircle(pos_x,pos_y,6,0);
  tft.drawCircle(pos_x,pos_y,5,0);
  tft.drawLine(pos_x,pos_y-3, pos_x, pos_y+3, 0);
  tft.drawLine(pos_x-3,pos_y, pos_x+3, pos_y, 0);

  pix_max=0;

}



void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal) {

  int colorTemp;

  for (int y=0; y<rows; y++) {
    for (int x=0; x<cols; x++) {
      float val = get_point(p, rows, cols, x, y);
      if(val >= MAXTEMP) colorTemp = MAXTEMP;
      else if(val <= MINTEMP) colorTemp = MINTEMP;
      else colorTemp = val;

      
      uint8_t colorIndex = map(colorTemp, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);

      //draw the pixels!
      uint16_t color;
      color = val * 2;
      tft.fillRect(boxWidth * x, boxHeight * y, boxWidth, boxHeight, camColors[colorIndex]);

  
      if (pix_max<val){

        pix_max =val;  

        pos_x=boxWidth * x;

        pos_y=boxHeight * y;

      }

      if (showVal) {

        tft.setCursor(boxWidth * y + boxWidth/2 - 12, 40 + boxHeight * x + boxHeight/2 - 4);
        tft.setTextColor(WHITE);  tft.setTextSize(1);
        tft.print(val,1);

      }

    } 

  }

}





#include <Arduino.h>

float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y,float f);
void get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols,

                      int8_t x, int8_t y);

void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols,
int8_t x, int8_t y);

float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
float *dest, uint8_t dest_rows, uint8_t dest_cols);



float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {

  if (x < 0)
    x = 0;

  if (y < 0)
    y = 0;

  if (x >= cols)
    x = cols - 1;

  if (y >= rows)
    y = rows - 1;

  return p[y * cols + x];

}



void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y,

               float f) {

  if ((x < 0) || (x >= cols))

    return;

  if ((y < 0) || (y >= rows))

    return;

  p[y * cols + x] = f;

}



// src is a grid src_rows * src_cols

// dest is a pre-allocated grid, dest_rows*dest_cols

void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,

                       float *dest, uint8_t dest_rows, uint8_t dest_cols) {

  float mu_x = (src_cols - 1.0) / (dest_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dest_rows - 1.0);
  float adj_2d[16]; // matrix for storing adjacents


  for (uint8_t y_idx = 0; y_idx < dest_rows; y_idx++) {
    for (uint8_t x_idx = 0; x_idx < dest_cols; x_idx++) {

      float x = x_idx * mu_x;
      float y = y_idx * mu_y;
      // Serial.print("("); Serial.print(y_idx); Serial.print(", ");
      // Serial.print(x_idx); Serial.print(") = "); Serial.print("(");
      // Serial.print(y); Serial.print(", "); Serial.print(x); Serial.print(") =
      // ");

      get_adjacents_2d(src, adj_2d, src_rows, src_cols, x, y);

      /*

      Serial.print("[");

      for (uint8_t i=0; i<16; i++) {

        Serial.print(adj_2d[i]); Serial.print(", ");

      }

      Serial.println("]");

      */

      float frac_x = x - (int)x; // we only need the ~delta~ between the points
      float frac_y = y - (int)y; // we only need the ~delta~ between the points
      float out = bicubicInterpolate(adj_2d, frac_x, frac_y);
      // Serial.print("\tInterp: "); Serial.println(out);
      set_point(dest, dest_rows, dest_cols, x_idx, y_idx, out);

    }

  }

}



// p is a list of 4 points, 2 to the left, 2 to the right

float cubicInterpolate(float p[], float x) {

  float r = p[1] + (0.5 * x *

                    (p[2] - p[0] +

                     x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] +

                          x * (3.0 * (p[1] - p[2]) + p[3] - p[0]))));

 

  return r;

}



// p is a 16-point 4x4 array of the 2 rows & columns left/right/above/below

float bicubicInterpolate(float p[], float x, float y) {

  float arr[4] = {0, 0, 0, 0};

  arr[0] = cubicInterpolate(p + 0, x);
  arr[1] = cubicInterpolate(p + 4, x);
  arr[2] = cubicInterpolate(p + 8, x);
  arr[3] = cubicInterpolate(p + 12, x);
  return cubicInterpolate(arr, y);

}



// src is rows*cols and dest is a 4-point array passed in already allocated!

void get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols,

                      int8_t x, int8_t y) {

  // Serial.print("("); Serial.print(x); Serial.print(", "); Serial.print(y);
  // Serial.println(")");
  // pick two items to the left

  dest[0] = get_point(src, rows, cols, x - 1, y);
  dest[1] = get_point(src, rows, cols, x, y);
  // pick two items to the right
  dest[2] = get_point(src, rows, cols, x + 1, y);
  dest[3] = get_point(src, rows, cols, x + 2, y);

}



// src is rows*cols and dest is a 16-point array passed in already allocated!
void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols,

                      int8_t x, int8_t y) {

  // Serial.print("("); Serial.print(x); Serial.print(", "); Serial.print(y);

  // Serial.println(")");

  float arr[4];

  for (int8_t delta_y = -1; delta_y < 3; delta_y++) { // -1, 0, 1, 2

    float *row = dest + 4 * (delta_y + 1); // index into each chunk of 4

    for (int8_t delta_x = -1; delta_x < 3; delta_x++) { // -1, 0, 1, 2

      row[delta_x + 1] = get_point(src, rows, cols, x + delta_x, y + delta_y);

    }

  }

}



void colorbar(){



  for (int i=0;i<240;i++){

    tft.fillRect(i, 280, 1,24, camColors[(int)(i*1.05)]);

  }

  tft.setTextColor(COLOR_RGB565_WHITE);
  tft.setCursor(0,285);  
  tft.setTextSize(2);
  tft.print(MINTEMP);
  tft.print("C");
  tft.setCursor(202,285);  
  tft.print(MAXTEMP); 
  tft.print("C"); 

 }