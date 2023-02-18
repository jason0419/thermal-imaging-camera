#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <TFT_eSPI.h>
#include "colormap.h"
#include "header.h"

#define EMMISIVITY 0.98
#define INTERPOLATE false

// #define C_BLUE Display.color565(0,0,255)
// #define C_RED Display.color565(255,0,0)
// #define C_GREEN Display.color565(0,255,0)
// #define C_WHITE Display.color565(255,255,255)
// #define C_BLACK Display.color565(0,0,0)
// #define C_LTGREY Display.color565(200,200,200)
// #define C_DKGREY Display.color565(80,80,80)
// #define C_GREY Display.color565(127,127,127)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
paramsMLX90640 mlx90640;

TFT_eSPI Display = TFT_eSPI();

// Added for measure Temp
boolean measure = true;
float centerTemp;
unsigned long tempTime = millis();
unsigned long tempTime2 = 0;

// start with some initial colors
float minTemp = 15.0;
float maxTemp = 35.0;


// variables for interpolated colors
byte red, green, blue;

// variables for row/column interpolation
float intPoint, val, a, b, c, d, ii;
int x, y, i, j;

// Output size
#define OUTPUT_SCALE 2

#define O_WIDTH 32 * OUTPUT_SCALE
#define O_HEIGHT 24 * OUTPUT_SCALE

// array for the 32 x 24 measured tempValues
static float tempValues[32*24];
// float interpolatedTempValues[32*OUTPUT_SCALE*24*OUTPUT_SCALE];


// float **interpolated = NULL;
// float *interpolatedTempValues = NULL;
uint16_t *imageData = NULL;

void setup_display(){
  pinMode(TFT_DC, OUTPUT);
  SPI.begin();
  SPI.setFrequency(80000000L);
  Display.begin();
  Display.fillScreen(TFT_BLACK);
}

void setup_mlx90640(){
  Wire.begin();
  Wire.setClock(400000); // Increase I2C clock speed to 400kHz
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring.");
  }
  else {
    Serial.println("MLX90640 online!");
  }
  // Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) Serial.println("Parameter extraction failed");
  // Set refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x05); // Set rate to 8Hz effective - Works at 800kHz
  // Once EEPROM has been read at 400kHz we can increase
  Wire.setClock(800000);
}

void setup() {
  Serial.begin(115200);
  setup_display();
  setup_mlx90640();

  // // Prepare interpolated array
  // interpolated = (float **)malloc(O_HEIGHT * sizeof(float *));
  // for (int i=0; i<O_HEIGHT; i++) {
  //   interpolated[i] = (float *)malloc(O_WIDTH * sizeof(float));
  // }

  // Prepare interpolated array
  // interpolatedTempValues = (float *)malloc(O_WIDTH * O_HEIGHT * sizeof(float));

  // Prepare imageData array
  imageData = (uint16_t *)malloc(O_WIDTH * O_HEIGHT * sizeof(uint16_t));

  // get the cutoff points for the color interpolation routines
  // note this function called when the temp scale is changed
  setAbcd();
  drawLegend();
}


void loop() {
  tempTime = millis();
  readTempValues();
  filterTempValues(0.5);
  setTempScale();
  drawPicture();
  drawMeasurement();
}


// Read pixel data from MLX90640.
void readTempValues() {
  for (byte x = 0 ; x < 2 ; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
  }
}

void filterTempValues(float alpha){
  static float previous_tempValues[32*24];
  for (int i = 0; i < 32*24; i++){
    tempValues[i] = previous_tempValues[i] * (1-alpha) + tempValues[i] * alpha;
    previous_tempValues[i] = tempValues[i];
  }
}

// int row;
// float temp, temp2;

// void interpolate() {
//   for (row=0; row<24; row++) {
//     for (x=0; x<O_WIDTH; x++) {
//       temp  = tempValues[(31 - (x/7)) + (row*32) + 1];
//       temp2 = tempValues[(31 - (x/7)) + (row*32)];
//       interpolated[row*7][x] = lerp(temp, temp2, x%7/7.0);
//     }
//   }
//   for (x=0; x<O_WIDTH; x++) {
//     for (y=0; y<O_HEIGHT; y++) {
//       temp  = interpolated[y-y%7][x];
//       temp2 = interpolated[min((y-y%7)+7, O_HEIGHT-7)][x];
//       interpolated[y][x] = lerp(temp, temp2, 1);//y%7/7.0);
//     }
//   }
// }

// Linear interpolation
float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

// void interpolation(){

//   for (int col = 0; col < 32; col++){
//     for (int row = 0; row < 24; row++){
//       interpolatedTempValues[row*OUTPUT_SCALE*32+col*OUTPUT_SCALE] = tempValues[row*32+col];
//     }
//   }

//   for (int row = 0; row < 24; row++){
//     for (int col = 0; col < 32*OUTPUT_SCALE; col++){
//       if(col%OUTPUT_SCALE != 0){
//         interpolatedTempValues[row*OUTPUT_SCALE*32+col*OUTPUT_SCALE] = map(((col/OUTPUT_SCALE) - int(col/OUTPUT_SCALE))*100, 0, 100, tempValues[int(col/OUTPUT_SCALE)]*100, tempValues[int(col/OUTPUT_SCALE)+1]*100) / 100.0;
//       }
//     }
//   }

// }

int map_f(float in, float a, float b)
{
    if (in < a)
        return 0;

    if (in > b)
        return 255;

    return (int)(in - a) * 255 / (b - a);
}

//Transform 32*24 to 320 * 240 pixel
void interpolation(float *data, uint16_t *out){
  for (uint8_t h = 0; h < 24; h++){
    for (uint8_t w = 0; w < 32; w++){
      out[h * OUTPUT_SCALE * O_WIDTH + w * OUTPUT_SCALE] = int(data[h * 32 + w]*100);
    }
  }
  for (int h = 0; h < O_HEIGHT; h += OUTPUT_SCALE){
    for (int w = 1; w < (O_WIDTH-OUTPUT_SCALE); w += OUTPUT_SCALE){
      for (int i = 0; i < (OUTPUT_SCALE-1); i++){
        out[h * O_WIDTH + w + i] = (out[h * O_WIDTH + w - 1] * ((OUTPUT_SCALE-1) - i) + out[h * O_WIDTH + w + (OUTPUT_SCALE-1)] * (i + 1)) / OUTPUT_SCALE;
      }
    }
    for (int i = 0; i < (OUTPUT_SCALE-1); i++){
      out[h * O_WIDTH + (O_WIDTH-OUTPUT_SCALE+1) + i] = out[h * O_WIDTH + (O_WIDTH-OUTPUT_SCALE)];
    }
  }
  for (int w = 0; w < O_WIDTH; w++){
    for (int h = 1; h < (O_HEIGHT-OUTPUT_SCALE); h += OUTPUT_SCALE){
      for (int i = 0; i < (OUTPUT_SCALE-1); i++){
        out[(h + i) * O_WIDTH + w] = (out[(h - 1) * O_WIDTH + w] * ((OUTPUT_SCALE-1) - i) + out[(h + (OUTPUT_SCALE-1)) * O_WIDTH + w] * (i + 1)) / OUTPUT_SCALE;
      }
    }
    for (int i = 0; i < (OUTPUT_SCALE-1); i++){
      out[((O_HEIGHT-OUTPUT_SCALE+1) + i) * O_WIDTH + w] = out[(O_HEIGHT-OUTPUT_SCALE) * O_WIDTH + w];
    }
  }
  for (int h = 0; h < O_HEIGHT; h++){
    for (int w = 0; w < O_WIDTH; w++){
      out[h * O_WIDTH + w] = getColor(out[h * O_WIDTH + w]/100.0);
      // out[h * O_WIDTH + w] = getColor(10000.0);
    }
  }
}






void drawPicture() {
  if (INTERPOLATE) {
    // interpolate();
    interpolation(tempValues, imageData);
    // for (y=0; y<O_HEIGHT; y++) {
    //   for (x=0; x<O_WIDTH; x++) {
    //     // imageData[(y*O_WIDTH) + x] = getColor(interpolated[y][x]);
    //     imageData[(y*O_WIDTH) + x] = getColor(interpolatedTempValues[(y*O_WIDTH) + x]);
    //   }
    // }
    // for (y=0; y<O_HEIGHT; y++) {
    //   for (x=0; x<O_WIDTH; x++) {
    //     Display.fillRect(8 + x, 8 + y, 1, 1, imageData[y*O_WIDTH + x]);
    //   }
    // }    
    Display.pushImage(8, 8, O_WIDTH, O_HEIGHT, imageData);
  }
  else {
    for (y=0; y<24; y++) {
      for (x=0; x<32; x++) {
        Display.fillRect(8 + x*7, 8 + y*7, 7, 7, getColor(tempValues[(31-x) + (y*32)]));
      }
    }
  }
}


uint16_t getColor(float val){
  uint8_t colorIndex = map(constrain(val, minTemp, maxTemp)*100, minTemp*100, maxTemp*100, 0, 255);
  return COLORMAP[colorIndex];
}

// // Get color for temp value.
// uint16_t getColor(float val) {
//   /*
//     pass in value and figure out R G B
//     several published ways to do this I basically graphed R G B and developed simple linear equations
//     again a 5-6-5 color display will not need accurate temp to R G B color calculation

//     equations based on
//     http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html

//   */

//   red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

//   if ((val > minTemp) & (val < a)) {
//     green = constrain(255.0 / (a - minTemp) * val - (255.0 * minTemp) / (a - minTemp), 0, 255);
//   }
//   else if ((val >= a) & (val <= c)) {
//     green = 255;
//   }
//   else if (val > c) {
//     green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
//   }
//   else if ((val > d) | (val < a)) {
//     green = 0;
//   }

//   if (val <= b) {
//     blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
//   }
//   else if ((val > b) & (val <= d)) {
//     blue = 0;
//   }
//   else if (val > d) {
//     blue = constrain(240.0 / (maxTemp - d) * val - (d * 240.0) / (maxTemp - d), 0, 240);
//   }

//   // use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
//   return Display.color565(red, green, blue);
// }


void setTempScale() {
  // minTemp = 255;
  // maxTemp = 0;

  // for (i = 0; i < 768; i++) {
  //   minTemp = min(minTemp, tempValues[i]);
  //   maxTemp = max(maxTemp, tempValues[i]);
  // }

  setAbcd();
  drawLegend();
}


// Function to get the cutoff points in the temp vs RGB graph.
void setAbcd() {
  a = minTemp + (maxTemp - minTemp) * 0.2121;
  b = minTemp + (maxTemp - minTemp) * 0.3182;
  c = minTemp + (maxTemp - minTemp) * 0.4242;
  d = minTemp + (maxTemp - minTemp) * 0.8182;
}


// Draw a legend.
void drawLegend() {
  float inc = (maxTemp - minTemp) / 224.0;
  j = 0;
  for (ii = minTemp; ii < maxTemp; ii += inc) {
    Display.drawFastVLine(8+ + j++, 292, 20, getColor(ii));
  }

  Display.setTextFont(2);
  Display.setTextSize(1);
  Display.setCursor(8, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(minTemp).substring(0, 5));

  Display.setCursor(192, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(maxTemp).substring(0, 5));

  Display.setTextFont(NULL);
}


// Draw a circle + measured value.
void drawMeasurement() {

  // Mark center measurement
  Display.drawCircle(120, 8+84, 3, TFT_WHITE);

  // Measure and print center temperature
  centerTemp = (tempValues[383 - 16] + tempValues[383 - 15] + tempValues[384 + 15] + tempValues[384 + 16]) / 4;
  Display.setCursor(86, 214);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextFont(2);
  Display.setTextSize(2);
  Display.print(String(centerTemp).substring(0, 5) + " °C");
}