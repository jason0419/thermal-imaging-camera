#ifndef HEADER_H
#define HEADER_H

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

#endif