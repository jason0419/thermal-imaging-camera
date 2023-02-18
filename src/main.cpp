#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <TFT_eSPI.h>
#include "image_color.h"
#include "header.h"

// #define MLX_I2C_ADDR 0x33
// #define TA_SHIFT 8 //Default shift for MLX90640 in open air
// float mlx90640To[768];
// paramsMLX90640 mlx90640;

// TFT_eSPI display = TFT_eSPI();

// #define MINTEMP 25
// #define MAXTEMP 37
// #define MLX_MIRROR 1 // Set 1 when the camera is facing the screen
// #define FILTER_ENABLE 1
// #define INTERPOLATION_ENABLE 1

// float frame[32 * 24]; // buffer for full frame of temperatures
// float *temp_frame = NULL;
// uint16_t *inter_p = NULL;

// void setup_display(){
//   display.begin();
//   display.setRotation(1);  // 0 & 2 Portrait. 1 & 3 landscape
//   display.fillScreen(TFT_RED);
// }

// //Returns true if the MLX90640 is detected on the I2C bus
// boolean mlx90640_isConnected()
// {
//   Wire.beginTransmission((uint8_t)MLX_I2C_ADDR);
//   if (Wire.endTransmission() != 0)
//     return (false); //Sensor did not ACK
//   return (true);
// }

// void setup_mlx90640(){

//   if (mlx90640_isConnected() == false)
//   {
//     Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
//     while (1);
//   }

//   //Get device parameters - We only have to do this once
//   int status;
//   uint16_t eeMLX90640[832];
//   status = MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
//   if (status != 0) Serial.println("Failed to load system parameters");
//   status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
//   if (status != 0) Serial.println("Parameter extraction failed");

//   //MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 2Hz
//   MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0x03); //Set rate to 4Hz
//   //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 64Hz
//   Serial.println("MLX90640 Connected");
// }


// void setup(void)
// {
//   Serial.begin(115200);

//   setup_display();

//   Wire.begin();
//   Wire.setClock(400000); //Increase I2C clock speed to 400kHz
//   setup_mlx90640();

//     // Serial.begin(115200);


//     // Serial.println(ESP.getFreeHeap());

//     // lcd.init();
//     // lcd.setRotation(0);
//     // display_ui();

//     // SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
//     // if (SD_init())
//     // {
//     //     Serial.println("SD init error");
//     // }
//     // appendFile(SD, "/temper.txt", "New Begin\n");

//     // //I2C init
//     // Wire.begin(I2C_SDA, I2C_SCL);
//     // byte error, address;

//     // Wire.beginTransmission(MLX_I2C_ADDR);
//     // error = Wire.endTransmission();

//     // if (error == 0)
//     // {
//     //     Serial.print("I2C device found at address 0x");
//     //     Serial.print(MLX_I2C_ADDR, HEX);
//     //     Serial.println("  !");
//     // }
//     // else if (error == 4)
//     // {
//     //     Serial.print("Unknown error at address 0x");
//     //     Serial.println(MLX_I2C_ADDR, HEX);
//     // }

//     // Serial.println("Adafruit MLX90640 Simple Test");
//     // if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
//     // {
//     //     Serial.println("MLX90640 not found!");
//     //     while (1)
//     //         delay(10);
//     // }

//     // //mlx.setMode(MLX90640_INTERLEAVED);
//     // mlx.setMode(MLX90640_CHESS);
//     // mlx.setResolution(MLX90640_ADC_18BIT);
//     // mlx90640_resolution_t res = mlx.getResolution();
//     // mlx.setRefreshRate(MLX90640_16_HZ);
//     // mlx90640_refreshrate_t rate = mlx.getRefreshRate();
//     // Wire.setClock(1000000); // max 1 MHz

//     // Serial.println(ESP.getFreeHeap());
//     // //image init
//     // inter_p = (uint16_t *)malloc(320 * 240 * sizeof(uint16_t));
//     // if (inter_p == NULL)
//     // {
//     //     Serial.println("Malloc error");
//     // }

//     // for (int i = 0; i < 320 * 240; i++)
//     // {
//     //     *(inter_p + i) = 0x480F;
//     // }

//     // Serial.println(ESP.getFreeHeap());
//     // //frame init
//     // temp_frame = (float *)malloc(32 * 24 * sizeof(float));
//     // if (inter_p == NULL)
//     // {
//     //     Serial.println("Malloc error");
//     // }

//     // for (int i = 0; i < 32 * 24; i++)
//     // {
//     //     temp_frame[i] = MINTEMP;
//     // }

//     // Serial.println("All init over.");
// }

// uint32_t runtime = 0;
// int fps = 0;
// float max_temp = 0.0;
// int record_index = 0;

// void loop()
// {
//     // //获取一帧
//     // //Get a frame
//     // if (mlx.getFrame(frame) != 0)
//     // {
//     //     Serial.println("Get frame failed");
//     //     return;
//     // }

//     // //和上次结果平均，滤波
//     // //Filter temperature data
//     // filter_frame(frame, temp_frame);

//     // //快排
//     // qusort(frame, 0, 32 * 24 - 1);
//     // max_temp += frame[767];

//     // if (INTERPOLATION_ENABLE == 1)
//     // {
//     //     //温度矩阵转换图像矩阵，将32*24插值到320*240
//     //     //Display with 320*240 pixel
//     //     interpolation(temp_frame, inter_p);
//     //     lcd.pushImage(0, 0, 320, 240, (lgfx::rgb565_t *)inter_p);
//     // }
//     // else
//     // {
//     //     //Display with 32*24 pixel
//     //     for (uint8_t h = 0; h < 24; h++)
//     //     {
//     //         for (uint8_t w = 0; w < 32; w++)
//     //         {
//     //             uint8_t colorIndex = map_f(temp_frame[h * 32 + w], MINTEMP, MAXTEMP);
//     //             lcd.fillRect(10 * w, 10 * h, 10, 10, camColors[colorIndex]);
//     //         }
//     //     }
//     // }

//     // fps++;

//     // if (get_pos(pos))
//     // {
//     //     Serial.println((String) "x=" + pos[0] + ",y=" + pos[1]);
//     //     if (pos[0] > 210 && pos[1] > 400 && pos[0] < 320 && pos[1] < 480)
//     //     {
//     //         touch_flag = 1;
//     //     }
//     // }

//     // if ((millis() - runtime) > 2000)
//     // {
//     //     lcd.fillRect(0, 280, 319, 79, TFT_BLACK);

//     //     lcd.setTextSize(4);
//     //     lcd.setCursor(0, 280);
//     //     lcd.setTextColor(camColors[map_f(max_temp / fps, MINTEMP, MAXTEMP)]);
//     //     lcd.println("  Max Temp:");
//     //     lcd.printf("  %6.1lf C", max_temp / fps);

//     //     lcd.setTextSize(1);
//     //     lcd.setTextColor(TFT_WHITE);
//     //     lcd.setCursor(0, 350);
//     //     lcd.printf("  FPS:%2.1lf", fps / 2.0);

//     //     if (touch_flag == 1)
//     //     {
//     //         touch_flag = 0;

//     //         lcd.fillRect(0, 380, 220, 80, TFT_BLACK);
//     //         lcd.setTextColor(TFT_WHITE);
//     //         lcd.setTextSize(1);
//     //         lcd.setCursor(10, 390);
//     //         lcd.printf("Record NO.%d to SD", record_index);

//     //         //Save Max temperture to sd card
//     //         char c[20];
//     //         sprintf(c, "[%d]\tT:%lf C\n", record_index++, max_temp / fps);
//     //         appendFile(SD, "/temper.txt", c);
//     //     }

//     //     runtime = millis();
//     //     max_temp = 0.0;
//     //     fps = 0;
//     // }
// }

// //消抖并翻转
// // //Filter temperature data and change camera direction
// // void filter_frame(float *in, float *out)
// // {
// //     if (MLX_MIRROR == 1)
// //     {
// //         for (int i = 0; i < 32 * 24; i++)
// //         {
// //             if (FILTER_ENABLE == 1)
// //                 out[i] = (out[i] + in[i]) / 2;
// //             else
// //                 out[i] = in[i];
// //         }
// //     }
// //     else
// //     {
// //         for (int i = 0; i < 24; i++)
// //             for (int j = 0; j < 32; j++)
// //             {
// //                 if (FILTER_ENABLE == 1)
// //                     out[32 * i + 31 - j] = (out[32 * i + 31 - j] + in[32 * i + j]) / 2;
// //                 else
// //                     out[32 * i + 31 - j] = in[32 * i + j];
// //             }
// //     }
// // }

// // //32*24插值10倍到320*240
// // //Transform 32*24 to 320 * 240 pixel
// // void interpolation(float *data, uint16_t *out)
// // {

// //     for (uint8_t h = 0; h < 24; h++)
// //     {
// //         for (uint8_t w = 0; w < 32; w++)
// //         {
// //             out[h * 10 * 320 + w * 10] = map_f(data[h * 32 + w], MINTEMP, MAXTEMP);
// //         }
// //     }
// //     for (int h = 0; h < 240; h += 10)
// //     {
// //         for (int w = 1; w < 310; w += 10)
// //         {
// //             for (int i = 0; i < 9; i++)
// //             {
// //                 out[h * 320 + w + i] = (out[h * 320 + w - 1] * (9 - i) + out[h * 320 + w + 9] * (i + 1)) / 10;
// //             }
// //         }
// //         for (int i = 0; i < 9; i++)
// //         {
// //             out[h * 320 + 311 + i] = out[h * 320 + 310];
// //         }
// //     }
// //     for (int w = 0; w < 320; w++)
// //     {
// //         for (int h = 1; h < 230; h += 10)
// //         {
// //             for (int i = 0; i < 9; i++)
// //             {
// //                 out[(h + i) * 320 + w] = (out[(h - 1) * 320 + w] * (9 - i) + out[(h + 9) * 320 + w] * (i + 1)) / 10;
// //             }
// //         }
// //         for (int i = 0; i < 9; i++)
// //         {
// //             out[(231 + i) * 320 + w] = out[230 * 320 + w];
// //         }
// //     }
// //     for (int h = 0; h < 240; h++)
// //     {
// //         for (int w = 0; w < 320; w++)
// //         {
// //             out[h * 320 + w] = camColors[out[h * 320 + w]];
// //         }
// //     }
// // }

// // //float to 0,255
// // int map_f(float in, float a, float b)
// // {
// //     if (in < a)
// //         return 0;

// //     if (in > b)
// //         return 255;

// //     return (int)(in - a) * 255 / (b - a);
// // }

// // //Quick sort
// // void qusort(float s[], int start, int end) //自定义函数 qusort()
// // {
// //     int i, j;        //定义变量为基本整型
// //     i = start;       //将每组首个元素赋给i
// //     j = end;         //将每组末尾元素赋给j
// //     s[0] = s[start]; //设置基准值
// //     while (i < j)
// //     {
// //         while (i < j && s[0] < s[j])
// //             j--; //位置左移
// //         if (i < j)
// //         {
// //             s[i] = s[j]; //将s[j]放到s[i]的位置上
// //             i++;         //位置右移
// //         }
// //         while (i < j && s[i] <= s[0])
// //             i++; //位置左移
// //         if (i < j)
// //         {
// //             s[j] = s[i]; //将大于基准值的s[j]放到s[i]位置
// //             j--;         //位置左移
// //         }
// //     }
// //     s[i] = s[0]; //将基准值放入指定位置
// //     if (start < i)
// //         qusort(s, start, j - 1); //对分割出的部分递归调用qusort()函数
// //     if (i < end)
// //         qusort(s, j + 1, end);
// // }

// // void display_ui()
// // {
// //     for (int i = 0; i < 256; i++)
// //         lcd.drawFastVLine(32 + i, 255, 20, camColors[i]);

// //     lcd.setTextSize(2);
// //     lcd.setCursor(5, 255);
// //     lcd.println((String) "" + MINTEMP);

// //     lcd.setCursor(290, 255);
// //     lcd.println((String) "" + MAXTEMP);

// //     lcd.fillRect(220, 380, 80, 80, TFT_GREEN);
// //     lcd.setCursor(230, 390);
// //     lcd.println("SAVE");

// // #ifdef WIFI_MODE
// //     lcd.setTextSize(1);
// //     lcd.setCursor(10, 460);
// //     lcd.println(WiFi.localIP());
// // #endif
// // }

// // int SD_init()
// // {

// //     if (!SD.begin(SD_CS))
// //     {
// //         Serial.println("Card Mount Failed");
// //         return 1;
// //     }
// //     uint8_t cardType = SD.cardType();

// //     if (cardType == CARD_NONE)
// //     {
// //         Serial.println("No SD card attached");
// //         return 1;
// //     }

// //     Serial.print("SD Card Type: ");
// //     if (cardType == CARD_MMC)
// //     {
// //         Serial.println("MMC");
// //     }
// //     else if (cardType == CARD_SD)
// //     {
// //         Serial.println("SDSC");
// //     }
// //     else if (cardType == CARD_SDHC)
// //     {
// //         Serial.println("SDHC");
// //     }
// //     else
// //     {
// //         Serial.println("UNKNOWN");
// //     }

// //     uint64_t cardSize = SD.cardSize() / (1024 * 1024);
// //     Serial.printf("SD Card Size: %lluMB\n", cardSize);
// //     listDir(SD, "/", 2);

// //     Serial.println("SD init over.");
// //     return 0;
// // }

// // void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
// // {
// //     Serial.printf("Listing directory: %s\n", dirname);

// //     File root = fs.open(dirname);
// //     if (!root)
// //     {
// //         Serial.println("Failed to open directory");
// //         return;
// //     }
// //     if (!root.isDirectory())
// //     {
// //         Serial.println("Not a directory");
// //         return;
// //     }

// //     File file = root.openNextFile();
// //     while (file)
// //     {
// //         if (file.isDirectory())
// //         {
// //             Serial.print("  DIR : ");
// //             Serial.println(file.name());
// //             if (levels)
// //             {
// //                 listDir(fs, file.name(), levels - 1);
// //             }
// //         }
// //         else
// //         {
// //             Serial.print("  FILE: ");
// //             Serial.print(file.name());
// //             Serial.print("  SIZE: ");
// //             Serial.println(file.size());
// //         }
// //         file = root.openNextFile();
// //     }
// // }

// // void readFile(fs::FS &fs, const char *path)
// // {
// //     Serial.printf("Reading file: %s\n", path);

// //     File file = fs.open(path);
// //     if (!file)
// //     {
// //         Serial.println("Failed to open file for reading");
// //         return;
// //     }

// //     Serial.print("Read from file: ");
// //     while (file.available())
// //     {
// //         Serial.write(file.read());
// //     }
// //     file.close();
// // }

// // void writeFile(fs::FS &fs, const char *path, const char *message)
// // {
// //     Serial.printf("Writing file: %s\n", path);

// //     File file = fs.open(path, FILE_WRITE);
// //     if (!file)
// //     {
// //         Serial.println("Failed to open file for writing");
// //         return;
// //     }
// //     if (file.print(message))
// //     {
// //         Serial.println("File written");
// //     }
// //     else
// //     {
// //         Serial.println("Write failed");
// //     }
// //     file.close();
// // }

// // void appendFile(fs::FS &fs, const char *path, const char *message)
// // {
// //     Serial.printf("Appending to file: %s\n", path);

// //     File file = fs.open(path, FILE_APPEND);
// //     if (!file)
// //     {
// //         Serial.println("Failed to open file for appending");
// //         return;
// //     }
// //     if (file.print(message))
// //     {
// //         Serial.println("Message appended");
// //     }
// //     else
// //     {
// //         Serial.println("Append failed");
// //     }
// //     file.close();
// // }

// // #ifdef WIFI_MODE

// // void handleRoot()
// // {
// //     server.send(200, "text/plain", "hello from esp32!");
// // }

// // #endif



/**
 * Based on: 
 * - https://github.com/wilhelmzeuschner/arduino_thermal_camera_with_sd_and_img_processing.
 * - https://github.com/sparkfun/SparkFun_MLX90640_Arduino_Example
 * 
 * Hardware:
 * - ESP32: https://www.espressif.com/en/products/hardware/esp32-devkitc/overview
 * - Sensor: https://shop.pimoroni.com/products/mlx90640-thermal-camera-breakout
 * - Display: https://www.amazon.de/gp/product/B07DPMV34R/, https://www.pjrc.com/store/display_ili9341.html
 */


#include <TFT_eSPI.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define EMMISIVITY 0.95
#define INTERPOLATE false

#define C_BLUE Display.color565(0,0,255)
#define C_RED Display.color565(255,0,0)
#define C_GREEN Display.color565(0,255,0)
#define C_WHITE Display.color565(255,255,255)
#define C_BLACK Display.color565(0,0,0)
#define C_LTGREY Display.color565(200,200,200)
#define C_DKGREY Display.color565(80,80,80)
#define C_GREY Display.color565(127,127,127)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
paramsMLX90640 mlx90640;


TFT_eSPI Display = TFT_eSPI();


// Added for measure Temp
boolean measure = true;
float centerTemp;
unsigned long tempTime = millis();
unsigned long tempTime2 = 0;

// start with some initial colors
float minTemp = 25.0;
float maxTemp = 35.0;


// variables for interpolated colors
byte red, green, blue;

// variables for row/column interpolation
float intPoint, val, a, b, c, d, ii;
int x, y, i, j;


// array for the 32 x 24 measured tempValues
static float tempValues[32*24];

// Output size
#define O_WIDTH 224
#define O_HEIGHT 168
#define O_RATIO O_WIDTH/32

float **interpolated = NULL;
uint16_t *imageData = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello.");

  // Connect thermal sensor.
  Wire.begin();
  Wire.setClock(400000); // Increase I2C clock speed to 400kHz
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring.");
  }
  else {
    Serial.println("MLX90640 online!");
  }
  // Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) Serial.println("Parameter extraction failed");
  // Set refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x05); // Set rate to 8Hz effective - Works at 800kHz
  // Once EEPROM has been read at 400kHz we can increase
  Wire.setClock(800000);

  // Set up Display.
  pinMode(TFT_DC, OUTPUT);
  SPI.begin();
  SPI.setFrequency(80000000L);
  Display.begin();
  //Display.setRotation(3);
  Display.fillScreen(C_BLACK);


  // Prepare interpolated array
  interpolated = (float **)malloc(O_HEIGHT * sizeof(float *));
  for (int i=0; i<O_HEIGHT; i++) {
    interpolated[i] = (float *)malloc(O_WIDTH * sizeof(float));
  }

  // Prepare imageData array
  imageData = (uint16_t *)malloc(O_WIDTH * O_HEIGHT * sizeof(uint16_t));

  // get the cutoff points for the color interpolation routines
  // note this function called when the temp scale is changed
  setAbcd();
  drawLegend();
}


void loop() {
  tempTime = millis();
  
  readTempValues();
  setTempScale();
  drawPicture();
  drawMeasurement();
}


// Read pixel data from MLX90640.
void readTempValues() {
  for (byte x = 0 ; x < 2 ; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
  }
}


int row;
float temp, temp2;

void interpolate() {
  for (row=0; row<24; row++) {
    for (x=0; x<O_WIDTH; x++) {
      temp  = tempValues[(31 - (x/7)) + (row*32) + 1];
      temp2 = tempValues[(31 - (x/7)) + (row*32)];
      interpolated[row*7][x] = lerp(temp, temp2, x%7/7.0);
    }
  }
  for (x=0; x<O_WIDTH; x++) {
    for (y=0; y<O_HEIGHT; y++) {
      temp  = interpolated[y-y%7][x];
      temp2 = interpolated[min((y-y%7)+7, O_HEIGHT-7)][x];
      interpolated[y][x] = lerp(temp, temp2, 1);//y%7/7.0);
    }
  }
}


// Linear interpolation
float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}


void drawPicture() {
  if (INTERPOLATE) {
    interpolate();
    for (y=0; y<O_HEIGHT; y++) {
      for (x=0; x<O_WIDTH; x++) {
        imageData[(y*O_WIDTH) + x] = getColor(interpolated[y][x]);
      }
    }
    Display.pushImage(8, 8, O_WIDTH, O_HEIGHT, imageData);
  }
  else {
    for (y=0; y<24; y++) {
      for (x=0; x<32; x++) {
        Display.fillRect(8 + x*7, 8 + y*7, 7, 7, getColor(tempValues[(31-x) + (y*32)]));
      }
    }
  }
}



// Get color for temp value.
uint16_t getColor(float val) {
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed simple linear equations
    again a 5-6-5 color display will not need accurate temp to R G B color calculation

    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html

  */

  red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

  if ((val > minTemp) & (val < a)) {
    green = constrain(255.0 / (a - minTemp) * val - (255.0 * minTemp) / (a - minTemp), 0, 255);
  }
  else if ((val >= a) & (val <= c)) {
    green = 255;
  }
  else if (val > c) {
    green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
  }
  else if ((val > d) | (val < a)) {
    green = 0;
  }

  if (val <= b) {
    blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
  }
  else if ((val > b) & (val <= d)) {
    blue = 0;
  }
  else if (val > d) {
    blue = constrain(240.0 / (maxTemp - d) * val - (d * 240.0) / (maxTemp - d), 0, 240);
  }

  // use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
  return Display.color565(red, green, blue);
}


void setTempScale() {
  // minTemp = 255;
  // maxTemp = 0;

  // for (i = 0; i < 768; i++) {
  //   minTemp = min(minTemp, tempValues[i]);
  //   maxTemp = max(maxTemp, tempValues[i]);
  // }

  setAbcd();
  drawLegend();
}


// Function to get the cutoff points in the temp vs RGB graph.
void setAbcd() {
  a = minTemp + (maxTemp - minTemp) * 0.2121;
  b = minTemp + (maxTemp - minTemp) * 0.3182;
  c = minTemp + (maxTemp - minTemp) * 0.4242;
  d = minTemp + (maxTemp - minTemp) * 0.8182;
}


// Draw a legend.
void drawLegend() {
  float inc = (maxTemp - minTemp) / 224.0;
  j = 0;
  for (ii = minTemp; ii < maxTemp; ii += inc) {
    Display.drawFastVLine(8+ + j++, 292, 20, getColor(ii));
  }

  Display.setTextFont(2);
  Display.setTextSize(1);
  Display.setCursor(8, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(minTemp).substring(0, 5));

  Display.setCursor(192, 272);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.print(String(maxTemp).substring(0, 5));

  Display.setTextFont(NULL);
}


// Draw a circle + measured value.
void drawMeasurement() {

  // Mark center measurement
  Display.drawCircle(120, 8+84, 3, TFT_WHITE);

  // Measure and print center temperature
  centerTemp = (tempValues[383 - 16] + tempValues[383 - 15] + tempValues[384 + 15] + tempValues[384 + 16]) / 4;
  Display.setCursor(86, 214);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextFont(2);
  Display.setTextSize(2);
  Display.print(String(centerTemp).substring(0, 5) + " °C");
}