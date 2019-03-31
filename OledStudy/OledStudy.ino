#include <myIOT.h>
#include <Arduino.h>

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET LED_BUILTIN
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~ myIOT  ~~~~~~~~~~~~~~~~~~~~~
#define DEVICE_TOPIC "HomePi/Dvir/OLED_Clock"
#define ADD_MQTT_FUNC addiotnalMQTT
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// Start services ~~~~~~~~~~~~~~~~~~~~~~~~~~
myIOT iot(DEVICE_TOPIC);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char *incoming_msg) {
}
void put_text(byte size = 1, byte x = 0, byte y = 0, char *text = "") {
//  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(y, x);
  display.print(text);
}

void center_text(int i, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte shift = 0) {
  char *Lines[] = {line1, line2, line3, line4};
  display.clearDisplay();
  display.setTextSize(i);
  display.setTextColor(WHITE);
  byte line_space = pow(2, (2 + i));

  for (int n = 0; n < 4; n++) {
    if (strcmp(Lines[n], "") != 0) {
      int strLength = strlen(Lines[n]);
      display.setCursor((ceil)((21 / i - strLength) / 2 * (128 / (21 / i))) + shift,  line_space * n);
      display.print(Lines[n]);
    }
  }
  display.display();
}




void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker

  /*  size: 1 = 21X4
    /   size: 2 = 10X2
      size: 4 = 5 X1
  */
  //  display.clearDisplay();
  //    display.setTextSize(2);             // Normal 1:1 pixel scale
  //    display.setTextColor(WHITE);        // Draw white text
  //    display.setCursor(0, 0);
  //    display.print("Off");
  //    display.setCursor(50,0);
  //    display.setTextSize(1);
  //    display.print("Abcdefgg");
  //      display.display();
}

void loop() {
  char a[12];
  iot.return_clock(a);
  char b[12];
  iot.return_date(b);
  center_text(2,a, b, "","");

//  display.display();

  iot.looper();


  //  display.setCursor(0, 0);
  //  display.clearDisplay();
  //display.print(millis());
  //    display.display();
  //      display.startscrollright(0x00, 0x0F);
  //    delay(200);
  //    display.stopscroll();
  //    delay(100);
}
