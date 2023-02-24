#ifndef HEADER_H
#define HEADER_H

#define PIN_SDA 21
#define PIN_SCL 22

void setup_mlx90640();
void initialize_imageArray();
void readTempValues();
void filterTempValues(float alpha);
void convertTempValues();
void bilinear_interpolation(int16_t* input, uint16_t* output);
void setTempScale();
float get_maxTemp();
float get_minTemp();
float get_centerTemp();
bool button_pressed(int, int);

#endif