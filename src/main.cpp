#include <Arduino.h>
#include <Wire.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#include "colormap.h"
#include "header.h"
#include "common.h"

#define USE_DISPLAY

#ifdef USE_DISPLAY
#include "display.h"
#endif

#define PIN_BUTTON_1 34
#define PIN_BUTTON_2 33
#define PIN_BUTTON_3 35

float emmisivity = 0.98;

bool apply_interpolation = true;

bool apply_filter = true;
float filter_alpha = 0.5;

int output_scale = 7;
int output_width = 32 * output_scale;
int output_height = 24 * output_scale;

bool fixed_scale = true;
float scale_min = 25.0;
float scale_max = 35.0;

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
paramsMLX90640 mlx90640;

// array for the 32 x 24 measured tempValues_raw
float tempValues_raw[32*24];
int16_t tempValues[32*24];
uint16_t *imageData = NULL;

void setup_buttons(){
  pinMode(PIN_BUTTON_1, PULLUP);
  pinMode(PIN_BUTTON_2, PULLUP);
  pinMode(PIN_BUTTON_3, PULLUP);
}


void setup_mlx90640(){
  Wire.begin(PIN_SDA, PIN_SCL);
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


void initialize_imageArray(){
  imageData = (uint16_t *)malloc(output_width * output_height * sizeof(uint16_t));
}


void readTempValues() {
  for (byte x = 0 ; x < 2 ; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);

    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
      ESP.restart();
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    // Serial.print(status);Serial.print(", ");Serial.print(vdd);Serial.print(", ");Serial.print(Ta);Serial.print(", ");Serial.print(tr);Serial.println("");
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emmisivity, tr, tempValues_raw);
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
  for (int x=0; x<output_width-(output_scale-1); x++){
    for(int y=0; y<output_height-(output_scale-1); y++){

      // Step 1: Convert the input x and y coordinates to integer values that represent
      // the index of the nearest grid point in the input array
      int x0 = x / output_scale;
      int y0 = y / output_scale;

      // Step 2: Compute the four corner points in the input array that surround the
      // point to be interpolated, using the indices x0, x0+1, y0, and y0+1
      int16_t f00 = input[y0 * 32 + x0];
      int16_t f01 = input[(y0 + 1) * 32 + x0];
      int16_t f10 = input[y0 * 32 + x0 + 1];
      int16_t f11 = input[(y0 + 1) * 32 + x0 + 1];

      // Step 3: Compute the weights for the four corner points using the fractional
      // part of the x and y coordinates
      int wx = x - x0 * output_scale;
      int wy = y - y0 * output_scale;
      int w00 = (output_scale - wx) * (output_scale - wy);
      int w01 = (output_scale - wx) * wy;
      int w10 = wx * (output_scale - wy);
      int w11 = wx * wy;

      // Step 4: Compute the interpolated value using the weighted average of the
      // four corner points
      int16_t value = (f00 * w00 + f01 * w01 + f10 * w10 + f11 * w11) / (output_scale * output_scale);

      // Step 5: Store the interpolated value in the output array at the corresponding location
      output[y * (32 * output_scale) + x] = value;
    }
  }
  for (int i=0; i<output_width*output_height; i++){
      output[i] = getColor(output[i]/100, scale_min, scale_max);
  }
}

#ifdef USE_DISPLAY
void setTempScale() {
  if(!fixed_scale){
    scale_min = 255;
    scale_max = 0;
    for (int i = 0; i < 32*24; i++) {
      scale_min = min(scale_min, tempValues_raw[i]);
      scale_max = max(scale_max, tempValues_raw[i]);
    }
  }
  drawLegend(scale_min, scale_max);
}
#endif


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


float get_centerTemp(){
  return (tempValues[383 - 16] + tempValues[383 - 15] + tempValues[384 + 15] + tempValues[384 + 16]) / 400.0;
}


void setup() {
  Serial.begin(115200);
  setup_mlx90640();
  generate_colormap(64, 128, 192);
  #ifdef USE_DISPLAY
  setup_display();
  drawLegend(scale_min, scale_max);
  #endif
  initialize_imageArray();
}


void loop() {

  static int buttonMode;
  readTempValues();
  if(apply_filter){
    filterTempValues(filter_alpha);
  }
  convertTempValues();
  #ifdef USE_DISPLAY
  setTempScale();
  if(apply_interpolation){
    bilinear_interpolation(tempValues, imageData);
    drawPicture_interpolated(output_width, output_height, output_scale, imageData);
  }
  else{
    drawPicture_pixelated(output_scale, tempValues_raw);
  }
  drawMeasurement(get_centerTemp(), get_maxTemp(), get_minTemp());
  drawButtonMode(buttonMode);
  #endif
  #ifndef USE_DISPLAY
  Serial.print("Center: ");Serial.println(get_centerTemp());
  Serial.print("Max:    ");Serial.println(get_maxTemp());
  Serial.print("Min:    ");Serial.println(get_minTemp());
  Serial.println("#################");
  #endif

  if(!analogRead(PIN_BUTTON_2)){
    delay(50);
    if(!analogRead(PIN_BUTTON_2)){
      buttonMode+=1;
      buttonMode = buttonMode%3;
      delay(50);
    }  
  }

  if (buttonMode==0){
    if(!analogRead(PIN_BUTTON_1)){
      delay(50);
      if(!analogRead(PIN_BUTTON_1)){
        fixed_scale = false;
        delay(50);
      }  
    }    
    if(!analogRead(PIN_BUTTON_3)){
      delay(50);
      if(!analogRead(PIN_BUTTON_3)){
        fixed_scale = true;
        scale_min = 25.0;
        scale_max = 35.0;
        delay(50);
      }  
    }    
  }

  if (buttonMode==1){
    if(!analogRead(PIN_BUTTON_1)){
      delay(50);
      if(!analogRead(PIN_BUTTON_1)){
        scale_min -= 1.0;
        delay(50);
      }  
    }    
    if(!analogRead(PIN_BUTTON_3)){
      delay(50);
      if(!analogRead(PIN_BUTTON_3)){
        scale_min = min(scale_min+1, scale_max-1);
        delay(50);
      }  
    }    
  }

  if (buttonMode==2){
    if(!analogRead(PIN_BUTTON_1)){
      delay(50);
      if(!analogRead(PIN_BUTTON_1)){
        scale_max = max(scale_min+1, scale_max-1);
        delay(50);
      }  
    }    
    if(!analogRead(PIN_BUTTON_3)){
      delay(50);
      if(!analogRead(PIN_BUTTON_3)){
        scale_max += 1.0;
        delay(50);
      }  
    }    
  }



}