#ifndef THERMALCAMERA1_H
#define THERMALCAMERA1_H

#include <Arduino.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

class ThermalCamera {
  public:
    ThermalCamera(); // Constructor
    void initialize(int pin_sda, int pin_scl, uint8_t _i2c_address);
    void get_temperature(float* _temperature);
    void set_emissivity(float _emissivity);
    void set_scale_min(float _scale_min);
    void set_scale_max(float _scale_max);
    void set_filter_alpha(float filter_alpha);
    void set_output_ratio(uint8_t _output_ratio);
    float get_emissivity();
    float get_scale_min();
    float get_scale_max();
    float get_filter_alpha();
    uint8_t get_output_ratio();
    void get_image_rgb565(uint16_t * imageData);
    void get_image_pixelated_rgb565(uint16_t * imageData);
    void get_image_jpeg(uint16_t * imageData);
    uint16_t getColor(float val, float min, float max);
    void hflip();
    float get_center_temperature();
    float get_max_temperature();
    float get_min_temperature();
    paramsMLX90640 mlx90640;
    
  private:
    uint8_t i2c_address;
    float emissivity = 0.98;
    float scale_min = 25.0;
    float scale_max = 35.0;
    uint8_t output_ratio = 7;
    uint16_t *colormap;
    void generate_colormap(uint8_t p0, uint8_t p1, uint8_t p2);
    float filter_alpha = 0.5;
    void filter_temperature();
    float *temperature;
};


#endif
