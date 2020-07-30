/* Example code for scrolling text and other text effects on MAX7219 LED dot matrix display with Arduino. More info: https://www.makerguides.com */

// Include the required Arduino libraries:
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

int i = 0;

textEffect_t texteffect[] =
{
  PA_PRINT,
  PA_SCAN_HORIZ,
  PA_SCROLL_LEFT,
  PA_WIPE,
  PA_SCROLL_UP_LEFT,
  PA_SCROLL_UP,
  PA_OPENING_CURSOR,
  PA_GROW_UP,
  PA_MESH,
  PA_SCROLL_UP_RIGHT,
  PA_BLINDS,
  PA_CLOSING,
  PA_RANDOM,
  PA_GROW_DOWN,
  PA_SCAN_VERT,
  PA_SCROLL_DOWN_LEFT,
  PA_WIPE_CURSOR,
  PA_DISSOLVE,
  PA_OPENING,
  PA_CLOSING_CURSOR,
  PA_SCROLL_DOWN_RIGHT,
  PA_SCROLL_RIGHT,
  PA_SLICE,
  PA_SCROLL_DOWN
};

// Define hardware type, size, and output pins:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

// Create a new instance of the MD_Parola class with hardware SPI connection:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Setup for software SPI:
// #define DATAPIN 2
// #define CLK_PIN 4
// MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  myDisplay.begin();
  myDisplay.setIntensity(5);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setPause(1000);
  myDisplay.setSpeed(100);
  myDisplay.displayClear();
  Serial.begin(9600);
}
int h = 0;
int m = 0;
int s = 0;
void clock_clac() {
  int secs = (int)(millis() / 1000);
  h = (int)(secs / 3600);
  m = (int)((secs - h * 3600) / 60);
  s = secs - h * 3600 - m * 60;
}

void loop() {
    if (myDisplay.displayAnimate()) {
      if (i < sizeof(texteffect)) {
        i++;
      }
      else {
        i = 0;
      }
//  myDisplay.displayAnimate();
  clock_clac();
  char clk[20];
  sprintf(clk, "%02d:%02d:%02d 26/07/2020", h, m, s);
  Serial.println(clk);
  myDisplay.displayText(clk, myDisplay.getTextAlignment(), myDisplay.getSpeed(), myDisplay.getPause(), texteffect[2], texteffect[7]);
//  delay(2000);
  myDisplay.displayReset();
  
    }
}
