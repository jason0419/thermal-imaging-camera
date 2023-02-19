#ifndef HEADER_H
#define HEADER_H
void readTempValues();
void filterTempValues(float);
void convertTempValues();
void bilinear_interpolation(int16_t* input, uint16_t* output);
// void interpolate(float *data, uint16_t *out);
void generate_colormap(uint8_t p0, uint8_t p1, uint8_t p2);
// float lerp(float, float, float);
void drawPicture();
uint16_t getColor(float);
void setTempScale();
void setAbcd();
void drawLegend();
void drawMeasurement();
float get_maxTemp();
float get_minTemp();
#endif