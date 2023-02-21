#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>
#include <TFT_eSPI.h>

extern TFT_eSPI Display;
extern TFT_eSprite image;

void setup_display();


void drawPicture_interpolated(int output_width, int output_height, int output_scale, uint16_t *imageData);
void drawPicture_pixelated(int output_scale, float * tempValues_raw);

void drawMeasurement(float centerTemp, float maxTemp, float minTemp) ;
void drawLegend(float min_value, float max_value) ;
void drawButtonMode(int buttonMode);


#endif