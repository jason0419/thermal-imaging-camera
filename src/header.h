#ifndef HEADER_H
#define HEADER_H
void readTempValues();
void filterTempValues(float);
void interpolate();
float lerp(float, float, float);
void drawPicture();
uint16_t getColor(float);
void setTempScale();
void setAbcd();
void drawLegend();
void drawMeasurement();
#endif