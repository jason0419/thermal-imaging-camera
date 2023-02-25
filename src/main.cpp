#include <Arduino.h>
#include <Wire.h> 
#include "header.h"
#include "common.h"

#define USE_DISPLAY

#ifdef USE_DISPLAY
#include "display.h"
#endif

#define PIN_BUTTON_1 32
#define PIN_BUTTON_2 25
#define PIN_BUTTON_3 26

float temperature[32*24];

int output_ratio = 7;
uint16_t* imageData = (uint16_t*)malloc(32*output_ratio*24*output_ratio*sizeof(uint16_t));

ThermalCamera thermalCamera = ThermalCamera();

void setup_buttons(){
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);
}



float get_maxTemp(){
  float output = -99999;
  for (int i = 0; i < 32*24; i++){
    if(temperature[i]>output) output = temperature[i];
  }
  return output;
}


float get_minTemp(){
  float output = 99999;
  for (int i = 0; i < 32*24; i++){
    if(temperature[i]<output) output = temperature[i];
  }
  return output;
}

float get_centerTemp(){
  return (temperature[383 - 16] + temperature[383 - 15] + temperature[384 + 15] + temperature[384 + 16]) / 4.0;
}

void setup() {
  Serial.begin(115200);
  thermalCamera.initialize(PIN_SDA, PIN_SCL, 0x33);
  thermalCamera.set_output_ratio(output_ratio);
  #ifdef USE_DISPLAY
  setup_display();
  drawLegend(thermalCamera.get_scale_min(), thermalCamera.get_scale_max());
  #endif
}


void loop() {
  static int buttonMode;
  thermalCamera.read_temperature(temperature);
  thermalCamera.hflip();
  #ifdef USE_DISPLAY
  drawLegend(thermalCamera.get_scale_min(), thermalCamera.get_scale_max());
  thermalCamera.get_image_rgb565(imageData);
  drawPicture_interpolated(32*output_ratio, 24*output_ratio, output_ratio, imageData);
  drawMeasurement(get_centerTemp(), get_maxTemp(), get_minTemp());
  drawButtonMode(buttonMode);
  drawInfo(String(thermalCamera.get_emissivity()));
  #endif
  #ifndef USE_DISPLAY
  Serial.print("Center: ");Serial.println(get_centerTemp());
  Serial.print("Max:    ");Serial.println(get_maxTemp());
  Serial.print("Min:    ");Serial.println(get_minTemp());
  Serial.println("#################");
  #endif

  if(button_pressed(PIN_BUTTON_2, 20)){
    buttonMode+=1;
    buttonMode = buttonMode%3;    
  }

  if (buttonMode==0){
    if(button_pressed(PIN_BUTTON_1, 20)){
      thermalCamera.set_emissivity(thermalCamera.get_emissivity()-0.01);
    }
    if(button_pressed(PIN_BUTTON_3, 20)){
      thermalCamera.set_emissivity(thermalCamera.get_emissivity()+0.01);
    }
  }

  if (buttonMode==1){
    if(button_pressed(PIN_BUTTON_1, 20)){
      thermalCamera.set_scale_min(thermalCamera.get_scale_min()-1);
    }
    if(button_pressed(PIN_BUTTON_3, 20)){
      thermalCamera.set_scale_min(thermalCamera.get_scale_min()+1);
    }
  }

  if (buttonMode==2){
    if(button_pressed(PIN_BUTTON_1, 20)){
      thermalCamera.set_scale_max(thermalCamera.get_scale_max()-1);
    }
    if(button_pressed(PIN_BUTTON_3, 20)){
      thermalCamera.set_scale_max(thermalCamera.get_scale_max()+1);
    }
  }

}

bool button_pressed(int pin, int delay_ms){
  if(!analogRead(pin)){
    delay(delay_ms);
    if(!analogRead(pin)){
      return true;
    }  
  }      
  return false;
}