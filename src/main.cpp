#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <TFT_eSPI.h>
#include "colormap.h"
#include "header.h"

#define EMMISIVITY 0.98
#define INTERPOLATE true

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
paramsMLX90640 mlx90640;

TFT_eSPI Display = TFT_eSPI();
TFT_eSprite image = TFT_eSprite(&Display);

// Added for measure Temp
boolean measure = true;
float centerTemp;

// start with some initial colors
float minTemp = 25.0;
float maxTemp = 35.0;

// Output size
#define OUTPUT_SCALE 7
#define OUTPUT_WIDTH 32 * OUTPUT_SCALE
#define OUTPUT_HEIGHT 24 * OUTPUT_SCALE

// array for the 32 x 24 measured tempValues_raw
float tempValues_raw[32*24];
int16_t tempValues[32*24];
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
  MLX90640_SetRefreshRate(MLX90640_address, 0x05);
  Wire.setClock(800000);
}

void setup() {
  Serial.begin(115200);
  setup_display();
  setup_mlx90640();

  // generate colormap with cutoff points
  generate_colormap(64, 128, 192);

  // Prepare imageData array
  imageData = (uint16_t *)malloc(OUTPUT_WIDTH * OUTPUT_HEIGHT * sizeof(uint16_t));
  
  drawLegend();
}


void loop() {
  readTempValues();
  filterTempValues(0.5);
  convertTempValues();
  setTempScale();
  bilinear_interpolation(tempValues, imageData);
  drawPicture();
  drawMeasurement();
}

// Read pixel data from MLX90640.
void readTempValues() {
  for (byte x = 0 ; x < 2 ; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    // Serial.print(status);Serial.print(", ");
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
      ESP.restart();
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    // Serial.print(vdd);Serial.print(", ");Serial.print(Ta);Serial.print(", ");Serial.print(tr);Serial.println("");
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues_raw);
    if((vdd<2.9) || (Ta>50)){
      ESP.restart();
    }
  }
}

void filterTempValues(float alpha){
  static float previous_tempValues_raw[32*24];
  for (int i = 0; i < 32*24; i++){
    tempValues_raw[i] = previous_tempValues_raw[i] * (1-alpha) + tempValues_raw[i] * alpha;
    previous_tempValues_raw[i] = tempValues_raw[i];
  }
}

void convertTempValues(){
  for (int i=0; i<32*24; i++){
    tempValues[i] = int(tempValues_raw[i]*100);
  }
}


void bilinear_interpolation(int16_t* input, uint16_t* output) {
  for (int x=0; x<OUTPUT_WIDTH-(OUTPUT_SCALE-1); x++){
    for(int y=0; y<OUTPUT_HEIGHT-(OUTPUT_SCALE-1); y++){

      // Step 1: Convert the input x and y coordinates to integer values that represent
      // the index of the nearest grid point in the input array
      int x0 = x / OUTPUT_SCALE;
      int y0 = y / OUTPUT_SCALE;

      // Step 2: Compute the four corner points in the input array that surround the
      // point to be interpolated, using the indices x0, x0+1, y0, and y0+1
      int16_t f00 = input[y0 * 32 + x0];
      int16_t f01 = input[(y0 + 1) * 32 + x0];
      int16_t f10 = input[y0 * 32 + x0 + 1];
      int16_t f11 = input[(y0 + 1) * 32 + x0 + 1];

      // Step 3: Compute the weights for the four corner points using the fractional
      // part of the x and y coordinates
      int wx = x - x0 * OUTPUT_SCALE;
      int wy = y - y0 * OUTPUT_SCALE;
      int w00 = (OUTPUT_SCALE - wx) * (OUTPUT_SCALE - wy);
      int w01 = (OUTPUT_SCALE - wx) * wy;
      int w10 = wx * (OUTPUT_SCALE - wy);
      int w11 = wx * wy;

      // Step 4: Compute the interpolated value using the weighted average of the
      // four corner points
      int16_t value = (f00 * w00 + f01 * w01 + f10 * w10 + f11 * w11) / (OUTPUT_SCALE * OUTPUT_SCALE);

      // Step 5: Store the interpolated value in the output array at the corresponding location
      output[y * (32 * OUTPUT_SCALE) + x] = value;
    }
  }
  for (int i=0; i<OUTPUT_WIDTH*OUTPUT_HEIGHT; i++){
      output[i] = getColor(output[i]/100);
  }
}

void drawPicture() {
  if (INTERPOLATE) {
    image.createSprite(OUTPUT_WIDTH-OUTPUT_SCALE+1,OUTPUT_HEIGHT-OUTPUT_SCALE+1);
    image.setSwapBytes(true);
    image.pushImage(0, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT, imageData);
    image.pushSprite(8, 8);
  }
  else {
    for (int y=0; y<24; y++) {
      for (int x=0; x<32; x++) {
        Display.fillRect(8 + x*7, 8 + y*7, 7, 7, getColor(tempValues_raw[(31-x) + (y*32)]));
      }
    }
  }
}


uint16_t getColor(float val){
  uint8_t colorIndex = map(constrain(val, minTemp, maxTemp)*100, minTemp*100, maxTemp*100, 0, 255);
  return COLORMAP[colorIndex];
}

void generate_colormap(uint8_t p0, uint8_t p1, uint8_t p2){
  static byte r, g, b;
  b = 255;
  for (int i = 0; i<=p0; i++){
    g = map(i, 0, p0, 0, 255);
    COLORMAP[i] = Display.color565(r,g,b);
  }
  for (int i = p0+1; i<=p1; i++){
    b = map(-i, -p1, -(p0+1), 0, 255);
    COLORMAP[i] = Display.color565(r,g,b);
  }
  for (int i = p1+1; i<=p2; i++){
    r = map(i, p1+1, p2, 0, 255);
    COLORMAP[i] = Display.color565(r,g,b);
  }
  for (int i = p2+1; i<=255; i++){
    g = map(-i, -255, -(p2+1), 0, 255);
    COLORMAP[i] = Display.color565(r,g,b);
  }    
}

void setTempScale() {
  // minTemp = 255;
  // maxTemp = 0;
  // for (i = 0; i < 768; i++) {
  //   minTemp = min(minTemp, tempValues_raw[i]);
  //   maxTemp = max(maxTemp, tempValues_raw[i]);
  // }

  drawLegend();
}

// Draw a legend.
void drawLegend() {
  float inc = (maxTemp - minTemp) / 224.0;
  int j = 0;
  for (float ii = minTemp; ii < maxTemp; ii += inc) {
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
  centerTemp = (tempValues[383 - 16] + tempValues[383 - 15] + tempValues[384 + 15] + tempValues[384 + 16]) / 400.0;
  Display.setCursor(8, 170);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextFont(2);
  Display.setTextSize(2);
  Display.print("Center: " + String(centerTemp).substring(0, 5) + " C");
  Display.setCursor(8, 200);
  Display.print("Max:    " + String(get_maxTemp()).substring(0, 5) + " C");
  Display.setCursor(8, 230);
  Display.print("Min:    " +String(get_minTemp()).substring(0, 5) + " C");
}

float get_maxTemp(){
  float output = -99999;
  for (int i = 0; i < 32*24; i++){
    if(tempValues[i]>output) output = tempValues[i];
  }
  return output/100.0;
}


float get_minTemp(){
  float output = 99999;
  for (int i = 0; i < 32*24; i++){
    if(tempValues[i]<output) output = tempValues[i];
  }
  return output/100.0;
}
