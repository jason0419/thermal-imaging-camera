#include "ThermalCamera.h"
#include <Wire.h>
#include <TFT_eSPI.h>  // for color565 function

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define TA_SHIFT 8 //Default shift for MLX90640 in open air


ThermalCamera::ThermalCamera(){}

void ThermalCamera::initialize(int pin_sda, int pin_scl, uint8_t _i2c_address){
  // I2C connection
  i2c_address = _i2c_address;
  Wire.begin(pin_sda, pin_scl);
  Wire.beginTransmission((uint8_t)i2c_address);
  if (Wire.endTransmission() != 0) Serial.println("MLX90640 not detected at default I2C address. Please check wiring.");
  else  Serial.println("MLX90640 connected.");

  // Get device parameters
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(i2c_address, eeMLX90640);
  if (status != 0) Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) Serial.println("Parameter extraction failed");
  
  // Set refresh rate and wire clock rate
  MLX90640_SetRefreshRate(i2c_address, 0x05);
  Wire.setClock(800000);

  // Initialize colormap
  colormap = (uint16_t *)malloc(256 * sizeof(uint16_t));
  generate_colormap(64, 128, 192);

  // Initialize tempature array
  temperature = (float *)malloc(32*24*sizeof(float));
}

void ThermalCamera::get_temperature(float* _temperature) {
  for (byte x = 0 ; x < 2 ; x++){
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(i2c_address, mlx90640Frame);

    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, temperature);
  }  
  filter_temperature();
  for(int i=0; i<32*24; i++){
    _temperature[i] = temperature[i];
  }
}

void ThermalCamera::filter_temperature(){
  static float previous_temperature[32*24];
  for (int i = 0; i < 32*24; i++){
    temperature[i] = previous_temperature[i] * (1-filter_alpha) + temperature[i] * filter_alpha;
    previous_temperature[i] = temperature[i];
  }
}

void ThermalCamera::set_emissivity(float _emissivity){
  emissivity = constrain(_emissivity, 0.01, 1.00);
}

void ThermalCamera::set_scale_min(float _scale_min){
  scale_min = min(_scale_min, scale_max-1);
}

void ThermalCamera::set_scale_max(float _scale_max){
  scale_max = max(_scale_max, scale_min+1);
}

void ThermalCamera::set_output_ratio(uint8_t _output_ratio){
  output_ratio = _output_ratio;
}

void ThermalCamera::set_filter_alpha(float _filter_alpha){
  filter_alpha = constrain(_filter_alpha, 0.01, 1.00);
}

float ThermalCamera::get_emissivity(){
  return emissivity;
}

float ThermalCamera::get_scale_min(){
  return scale_min;
}

float ThermalCamera::get_scale_max(){
  return scale_max;
}

float ThermalCamera::get_filter_alpha(){
  return filter_alpha;
}

uint8_t ThermalCamera::get_output_ratio(){
  return output_ratio;
}

void ThermalCamera::get_image_rgb565(uint16_t* imageData) {

  // interpolation
  for (int x=0; x<32*output_ratio-(output_ratio-1); x++){
    for(int y=0; y<24*output_ratio-(output_ratio-1); y++){

      // Step 1: Convert the input x and y coordinates to integer values that represent
      // the index of the nearest grid point in the input array
      int x0 = x / output_ratio;
      int y0 = y / output_ratio;

      // Step 2: Compute the four corner points in the input array that surround the
      // point to be interpolated, using the indices x0, x0+1, y0, and y0+1
      int16_t f00 = temperature[y0 * 32 + x0]*100;
      int16_t f01 = temperature[(y0 + 1) * 32 + x0]*100;
      int16_t f10 = temperature[y0 * 32 + x0 + 1]*100;
      int16_t f11 = temperature[(y0 + 1) * 32 + x0 + 1]*100;

      // Step 3: Compute the weights for the four corner points using the fractional
      // part of the x and y coordinates
      int wx = x - x0 * output_ratio;
      int wy = y - y0 * output_ratio;
      int w00 = (output_ratio - wx) * (output_ratio - wy);
      int w01 = (output_ratio - wx) * wy;
      int w10 = wx * (output_ratio - wy);
      int w11 = wx * wy;

      // Step 4: Compute the interpolated value using the weighted average of the
      // four corner points
      int16_t value = (f00 * w00 + f01 * w01 + f10 * w10 + f11 * w11) / (output_ratio * output_ratio);

      // Step 5: Convert to RGB565 and store in imageData
      imageData[y * (32 * output_ratio) + x] = getColor(value/100, scale_min, scale_max);
    }
  }
}


uint16_t ThermalCamera::getColor(float val, float min, float max){
    uint8_t colorIndex = map(constrain(val, min, max)*100, min*100, max*100, 0, 255);
    return colormap[colorIndex];
}

void ThermalCamera::generate_colormap(uint8_t p0, uint8_t p1, uint8_t p2){
    TFT_eSPI tft; // for color565 function
    static byte r, g, b;
    b = 255;
    for (int i = 0; i<=p0; i++){
        g = map(i, 0, p0, 0, 255);
        colormap[i] = tft.color565(r,g,b);
    }
    for (int i = p0+1; i<=p1; i++){
        b = map(-i, -p1, -(p0+1), 0, 255);
        colormap[i] = tft.color565(r,g,b);
    }
    for (int i = p1+1; i<=p2; i++){
        r = map(i, p1+1, p2, 0, 255);
        colormap[i] = tft.color565(r,g,b);
    }
    for (int i = p2+1; i<=255; i++){
        g = map(-i, -255, -(p2+1), 0, 255);
        colormap[i] = tft.color565(r,g,b);
    }    
}

void ThermalCamera::hflip(){
  for (int i=0; i<24; i++){
    float temp;
    for(int j=0; j<16; j++){
      temp = temperature[i*32+j];
      temperature[i*32+j] = temperature[i*32+31-j];
      temperature[i*32+31-j] = temp;
    }
  }
}


float ThermalCamera::get_center_temperature(){
  return (temperature[383 - 16] + temperature[383 - 15] + temperature[384 + 15] + temperature[384 + 16]) / 4.0;
}

float ThermalCamera::get_max_temperature(){
  float output = -99999;
  for (int i = 0; i < 32*24; i++){
    if(temperature[i]>output) output = temperature[i];
  }
  return output;  
}

float ThermalCamera::get_min_temperature(){
  float output = 99999;
  for (int i = 0; i < 32*24; i++){
    if(temperature[i]<output) output = temperature[i];
  }
  return output;  
}