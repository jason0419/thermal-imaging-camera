#include <Arduino.h>
#include <TFT_eSPI.h>  // for color565 function
#include "colormap.h"
#include "common.h"

TFT_eSPI tft; // for color565 function

uint16_t * COLORMAP=(uint16_t *)malloc(256 * sizeof(uint16_t));

uint16_t getColor(float val, float min, float max){
    uint8_t colorIndex = map(constrain(val, min, max)*100, min*100, max*100, 0, 255);
    return COLORMAP[colorIndex];
}

void generate_colormap(uint8_t p0, uint8_t p1, uint8_t p2){
    static byte r, g, b;
    b = 255;
    for (int i = 0; i<=p0; i++){
        g = map(i, 0, p0, 0, 255);
        COLORMAP[i] = tft.color565(r,g,b);
    }
    for (int i = p0+1; i<=p1; i++){
        b = map(-i, -p1, -(p0+1), 0, 255);
        COLORMAP[i] = tft.color565(r,g,b);
    }
    for (int i = p1+1; i<=p2; i++){
        r = map(i, p1+1, p2, 0, 255);
        COLORMAP[i] = tft.color565(r,g,b);
    }
    for (int i = p2+1; i<=255; i++){
        g = map(-i, -255, -(p2+1), 0, 255);
        COLORMAP[i] = tft.color565(r,g,b);
    }    
}