#include "display.h"
#include "common.h"

TFT_eSPI Display = TFT_eSPI();
TFT_eSprite image = TFT_eSprite(&Display);

void setup_display(){
  pinMode(TFT_DC, OUTPUT);
  SPI.begin();
  SPI.setFrequency(80000000L);
  Display.begin();
  Display.fillScreen(TFT_BLACK);
}

void drawPicture_interpolated(int output_width, int output_height, int output_scale, uint16_t *imageData) {

  image.createSprite(output_width-output_scale+1,output_height-output_scale+1);
  image.setSwapBytes(true);
  image.pushImage(0, 0, output_width, output_height, imageData);
  image.drawCircle(output_width/2, output_height/2, 3, TFT_WHITE);
  image.pushSprite(8, 18);

}


void drawMeasurement(float centerTemp, float maxTemp, float minTemp) {
  Display.setCursor(8, 180);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextFont(2);
  Display.setTextSize(2);
  Display.print("Center: " + String(centerTemp).substring(0, 5) + " C");
  Display.setCursor(8, 210);
  Display.print("Max:    " + String(maxTemp).substring(0, 5) + " C");
  Display.setCursor(8, 240);
  Display.print("Min:    " +String(minTemp).substring(0, 5) + " C");
}

// Draw a legend.
void drawLegend(float min_value, float max_value) {
  float inc = (max_value - min_value) / 224.0;
  int j = 0;
  for (float ii = min_value; ii < max_value; ii += inc) {
    Display.drawFastVLine(8+ + j++, 292, 20, thermalCamera.getColor(ii, min_value, max_value));
  }

  Display.setTextFont(2);
  Display.setTextSize(1);
  Display.setCursor(8, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(min_value).substring(0, 5));

  Display.setCursor(192, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(max_value).substring(0, 5));

  Display.setTextFont(NULL);
}

void drawButtonMode(int buttonMode){
  String mode_description[4] = {"Emissivity  ", "Scale_Min   ", "Scale_Max   ", "Filter Alpha"};
  Display.setTextFont(1);
  Display.setTextSize(1);
  Display.setCursor(8, 8);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(mode_description[buttonMode]);
}


void drawInfo(String info){
  Display.setTextFont(1);
  Display.setTextSize(1);
  Display.setCursor(200, 8);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(info);
}