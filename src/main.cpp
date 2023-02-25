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

bool apply_interpolation = true;

void setup_buttons(){
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);
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
  thermalCamera.get_temperature(temperature);
  thermalCamera.hflip();
  #ifdef USE_DISPLAY
  drawLegend(thermalCamera.get_scale_min(), thermalCamera.get_scale_max());
  if(apply_interpolation) thermalCamera.get_image_rgb565(imageData);
  else thermalCamera.get_image_pixelated_rgb565(imageData);
  drawPicture_interpolated(32*output_ratio, 24*output_ratio, output_ratio, imageData);
  drawMeasurement(thermalCamera.get_center_temperature(), thermalCamera.get_max_temperature(), thermalCamera.get_min_temperature());
  drawButtonMode(buttonMode);
  if(buttonMode==0) drawInfo(String(thermalCamera.get_emissivity()));
  else if (buttonMode==1) drawInfo(String(thermalCamera.get_scale_min()));
  else if (buttonMode==2) drawInfo(String(thermalCamera.get_scale_max()));
  else if (buttonMode==3) drawInfo(String(thermalCamera.get_filter_alpha()));
  else if (buttonMode==4){
    if(apply_interpolation) drawInfo("True ");
    else drawInfo("False");
  } 
  #endif
  #ifndef USE_DISPLAY
  Serial.print("Center: ");Serial.println(thermalCamera.get_center_temperature());
  Serial.print("Max:    ");Serial.println(thermalCamera.get_max_temperature());
  Serial.print("Min:    ");Serial.println(thermalCamera.get_min_temperature());
  Serial.println("#################");
  #endif

  if(button_pressed(PIN_BUTTON_2, 20)){
    buttonMode+=1;
    buttonMode = buttonMode%5;    
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

  if (buttonMode==3){
    if(button_pressed(PIN_BUTTON_1, 20)){
      thermalCamera.set_filter_alpha(thermalCamera.get_filter_alpha()-0.01);
    }
    if(button_pressed(PIN_BUTTON_3, 20)){
      thermalCamera.set_filter_alpha(thermalCamera.get_filter_alpha()+0.01);
    }
  }

  if (buttonMode==4){
    if(button_pressed(PIN_BUTTON_1, 20)){
      apply_interpolation=false;
    }
    if(button_pressed(PIN_BUTTON_3, 20)){
      apply_interpolation=true;
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