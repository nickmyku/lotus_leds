// LOTUS LEDS (2007 LOTUS ELISE)
// Written By: Nicholas Mykulowycz
// Date Created: Sept 8, 2018
// Last Updated: Sept 8, 2018
//
// Program adds cabin mood lighting to the car using NeoPixel strips (addressable LEDs)
// All settings and current system state can be veiwed/changed using a graphic OLED screen

#include <SHT1x.h>              // temp/humidity sensor library
#include <Wire.h>               // I2C
#include <EEPROM.h>             // EEPROM (persistent settings storage)
#include <Adafruit_GFX.h>       // standard graphics, needed for OLED
#include <Adafruit_SSD1306.h>   // OLED library
#include <Adafruit_NeoPixel.h>  // NeoPixel addressable LED library

#define NUM_SETTINGS  6 // number of entries in the settings menu

#define NUM_PIXELS 32        // number of neopixels in the strand

// Pin Defintions
#define OLED_RESET  4   // OLED display reset pin
#define VSA_PIN     9   // output pin for the VSA enable/disable button
#define NEO_PIN     8   // NeoPixel data pin
#define SELECT_BTN  10  // select button for changing categories
#define UP_BTN      11  // up button for adjusting settings
#define DN_BTN      12  // down button for adjusting settings

#define RED               1   // array index of red_val
#define GREEN             2   // array index of green_val
#define BLUE              3   // array index of blue_val
#define BRIGHTNESS        4   // array index of brightness value
#define SAVE              5   // array index of save to EEPROM



// OLED Display
Adafruit_SSD1306 display(OLED_RESET);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);


float scale_up = 0;
float scale_dn = 0;
const String SETTINGS[NUM_SETTINGS] =  {" ", "Red", "Green",  "Blue", "Brightness", "Save to EEPROM?"}; 
int values[NUM_SETTINGS] =             {0,   200,   40,       120,     0,            0}; 
int category = 0;
                              // 0 - blank
                              // 1 - red value
                              // 2 - green value
                              // 3 - blue value
                              // 4 - brightness
                              // 5 - save to EEPROM



// ----------------------------------------------------------
//                          SETUP
// ----------------------------------------------------------


void setup() {
  int brightness_val = 0;
  int brightness_max = 0;
  
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DN_BTN, INPUT_PULLUP);
  pinMode(SELECT_BTN, INPUT_PULLUP);

  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // initialize screen text
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // read EEPROM values for RGB values
  values[RED] = EEPROM.read(RED);
  values[GREEN] = EEPROM.read(GREEN);
  values[BLUE] = EEPROM.read(BLUE);

  updateOLED();

  // initialize NeoPixels
  strip.begin();
  
  // calculate the current brightness value
  // if red_val is the largest use that for the calculation of brightness
  if((values[RED] >= values[GREEN]) and (values[RED] >= values[BLUE])){
    brightness_max = values[RED];
  }
  else{
    // if green_val is the largest use that for calculation of brightness
    if((values[GREEN] >= values[RED]) and (values[GREEN] >= values[BLUE])){
      brightness_max = values[GREEN];
    }
    // otherwise the blue_val is the largest and that should be used for brightness calculation
    else{
      brightness_max = values[BLUE];
    }
  }
  
  // set brigtness to 1%
  scale_dn = 2.54/brightness_max;
  // scale down each of the RGB values
  values[RED] = values[RED] * scale_dn;
  values[GREEN] = values[GREEN] * scale_dn;
  values[BLUE] = values[BLUE] * scale_dn;
  
  // begin fade loop
  while(brightness_val < brightness_max){
    // calculate the current brightness value
    // if red_val is the largest use that for the calculation of brightness
    if((values[RED] >= values[GREEN]) and (values[RED] >= values[BLUE])){
      brightness_val = values[RED];
    }
    else{
      // if green_val is the largest use that for calculation of brightness
      if((values[GREEN] >= values[RED]) and (values[GREEN] >= values[BLUE])){
        brightness_val = values[GREEN];
      }
      // otherwise the blue_val is the largest and that should be used for brightness calculation
      else{
        brightness_val = values[BLUE];
      }
    }
    // calcualte a scale up multiplier that will increase brightness by 1%
    scale_up = 1 + (2.54/brightness_val);
    // scale up each of the RGB values
    values[RED] = values[RED] * scale_up;
    values[GREEN] = values[GREEN] * scale_up;
    values[BLUE] = values[BLUE] * scale_up;
    // write color values to all neopixels in strand
    for(int k=0; k<NUM_PIXELS; k++) {
      strip.setPixelColor(k,strip.Color(values[RED],values[GREEN],values[BLUE]));
    }
    strip.show();
    // wait a short while
    delay(50);
  }

  // read EEPROM values for RGB values (again)
  values[RED] = EEPROM.read(RED);
  values[GREEN] = EEPROM.read(GREEN);
  values[BLUE] = EEPROM.read(BLUE);
  // write color values to all neopixels in strand
  for(int i=0; i<NUM_PIXELS; i++) {
    strip.setPixelColor(i,strip.Color(values[RED],values[GREEN],values[BLUE]));
  }
  strip.show();   
  
}

// ----------------------------------------------------------
//                          LOOP
// ----------------------------------------------------------

void loop() {

  if(digitalRead(SELECT_BTN) == LOW){
    debounce(SELECT_BTN);
    if(category < (NUM_SETTINGS-1)){
      category++;
    }
    else{
      category=0;
    }
  }

  // function of UP_BTN and DN_BTN when the red_val green_val or blue_val category is selected
  if((category == RED) or (category == GREEN) or (category == BLUE)){
    if(digitalRead(UP_BTN) == LOW){
      // debounce button press
      debounce(UP_BTN);
      if(values[category] < 244){
        values[category] += 10;
      }
      else{
        values[category] = 254;
      }
    }
    if(digitalRead(DN_BTN) == LOW){
      // debounce button press
      debounce(DN_BTN);
      if(values[category] > 10){
        values[category] -= 10;
      }
      else{
        values[category] = 0;
      }
    }
  }

  // function of UP_BTN and DN_BTN when brightness category is selected
  if(category == BRIGHTNESS){
    // calculate the current brightness value
    // if red_val is the largest use that for the calculation of brightness
    if((values[RED] >= values[GREEN]) and (values[RED] >= values[BLUE])){
      values[category] = values[RED];
    }
    else{
      // if green_val is the largest use that for calculation of brightness
      if((values[GREEN] >= values[RED]) and (values[GREEN] >= values[BLUE])){
        values[category] = values[GREEN];
      }
      // otherwise the blue_val is the largest and that should be used for brightness calculation
      else{
        values[category] = values[BLUE];
      }
    }
    
    if(digitalRead(UP_BTN) == LOW){
      // debounce button press
      debounce(UP_BTN);
      // calcualte a scale up multiplier that will increase brightness by 10%
      scale_up = 1 + (25.4/values[category]);
      // ensure brightness will not exceed 100%
      if(values[category] < 228){
        values[RED] = values[RED] * scale_up;
        values[GREEN] = values[GREEN] * scale_up;
        values[BLUE] = values[BLUE] * scale_up;
      }
    }
    if(digitalRead(DN_BTN) == LOW){
      // debounce button press
      debounce(DN_BTN);
      // calculate a scale down multiplier that will decrease brightness by 10%
      scale_dn = 1 - (25.4/values[category]);
      // ensure brightness will not drop below 10%
      if(values[category] > 51){
        values[RED] = values[RED] * scale_dn;
        values[GREEN] = values[GREEN] * scale_dn;
        values[BLUE] = values[BLUE] * scale_dn;
      }
    }
  }

 

  // function of UP_BTN and DN_BTN when save category is selected
  if(category == SAVE){
    if(values[category] == 1){
      // inform user that it is being saved
      display.clearDisplay();   // clear old text from display
      display.setCursor(0,0);   // reset cursor
      display.println("Saving...");
      display.display();  // update display
      delay(300);
      // write values to EEPROM
      EEPROM.write(RED, values[RED]);
      EEPROM.write(GREEN, values[GREEN]);
      EEPROM.write(BLUE, values[BLUE]);
      // infrom user save was successful
      display.clearDisplay();   // clear old text from display
      display.setCursor(0,0);   // reset cursor
      display.println("Saved to EEPROM!");
      display.display();  // update display
      delay(1000);
      // reset the save bit
      values[category] = 0;
      category = 0;
    }
    if(digitalRead(UP_BTN) == LOW){
      // debounce button press
      debounce(UP_BTN);
      values[category] = 1;
    }
    if(digitalRead(DN_BTN) == LOW){
      // debounce button press
      debounce(DN_BTN);
      values[category] = 0;
    }
  }

  
  // values[RED] is red value fro 0 to 254
  // values[GREEN] is green value fro 0 to 254
  // values[BLUE] is blue value fro 0 to 254

  for(int i=0; i<NUM_PIXELS; i++) {
    strip.setPixelColor(i,strip.Color(values[RED],values[GREEN],values[BLUE]));
  }
  strip.show();

  updateOLED();  
  
  //delay(100);  // wait before running loop again

}

// ----------------------------------------------------------
//                        FUNCTIONS
// ----------------------------------------------------------

// function for debouncing button presses
void debounce(int pin_num){
  while(digitalRead(pin_num) == LOW){
    delay(100);
  }
}


// function for updating the OLED Display used for sensor values and adjustmens
void updateOLED() {
  display.clearDisplay();   // clear old text from display
  display.setCursor(0,0);   // reset cursor

  // display the red percentage
  display.print(SETTINGS[RED]);
  display.print(": ");
  display.print(map(values[RED],0,254,0,100));
  display.println(" %");  

  // display the green percentage
  display.print(SETTINGS[GREEN]);
  display.print(": ");
  display.print(map(values[GREEN],0,254,0,100));
  display.println(" %");  

  // display the blue precentage
  display.print(SETTINGS[BLUE]);
  display.print(": ");
  display.print(map(values[BLUE],0,254,0,100));
  display.println(" %");  


  display.println(" ");

  
  switch (category) {
    case RED: // red intensity
      display.print(SETTINGS[RED]);
      display.print(": ");
      display.print(map(values[RED],0,254,0,100));
      display.println(" %");
      break;
    case GREEN: // green intensity
      display.print(SETTINGS[GREEN]);
      display.print(": ");
      display.print(map(values[GREEN],0,254,0,100));
      display.println(" %");
      break;
    case BLUE: // blue intensity
      display.print(SETTINGS[BLUE]);
      display.print(": ");
      display.print(map(values[BLUE],0,254,0,100));
      display.println(" %");
      break;
    case BRIGHTNESS: // brightness 
      display.print(SETTINGS[BRIGHTNESS]);
      display.print(": ");
      display.print(map(values[BRIGHTNESS],0,254,0,100));
      display.println(" %");
      break;
    case SAVE: // save
      display.print(SETTINGS[SAVE]);
      display.print(" ");
      if(values[SAVE] == 0){
        display.println("No");
      }
      else{
        display.println("Yes");
      }
      break;
    default:
      break;
  }
  
  
  display.display();  // update display
}
