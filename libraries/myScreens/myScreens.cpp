
#include <Arduino.h>
#include <myScreens.h>

// const int displayRows = 4;
// char timeStamp[50];
// char dateStamp[50];
// char bmsg[2][50];
// long bmsg_t = 0;

// #define SCREEN_WIDTH 128
// #if displayRows == 4
// #define SCREEN_HEIGHT 64 // 32 2rows or 64 4 rows
// #else if displayRows == 2
// #define SCREEN_HEIGHT 632 // 32 2rows or 64 4 rows
// #endif

// #define OLED_RESET LED_BUILTIN

// long swapLines_counter = 0;

// // char bmsg[2][50];
// // long bmsg_t = 0;
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// void startOLED()
// {
//         display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//         display.clearDisplay();
// }
// void OLED_CenterTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0, byte y_shift = 0)
// {
//         char *Lines[] = {line1, line2, line3, line4};
//         display.clearDisplay();
//         display.setTextSize(char_size);
//         display.setTextColor(WHITE);
//         byte line_space = pow(2, (2 + char_size));

//         for (int n = 0; n < 4; n++)
//         {
//                 if (strcmp(Lines[n], "") != 0)
//                 {
//                         int strLength = strlen(Lines[n]);
//                         display.setCursor((ceil)((21 / char_size - strLength) / 2 * (128 / (21 / char_size))) + x_shift, line_space * n + y_shift);
//                         display.print(Lines[n]);
//                 }
//         }
//         display.display();
// }
// void OLED_SideTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "")
// {
//         char *Lines[] = {line1, line2, line3, line4};
//         display.clearDisplay();
//         display.setTextSize(char_size);
//         byte line_space = pow(2, (2 + char_size));

//         if (strcmp(line3, "") == 0 && strcmp(line4, "") == 0)
//         { // for ON state only - 2rows
//                 for (int n = 0; n < 2; n++)
//                 {
//                         if (strcmp(Lines[n], "") != 0)
//                         {
//                                 if (n == 1)
//                                 { // Clock line
//                                         display.setTextSize(char_size);
//                                         display.setTextColor(WHITE);
//                                         int strLength = strlen(Lines[n]);
//                                         display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * (n + 1) - 3);
//                                         display.print(Lines[n]);
//                                 }
//                                 else
//                                 { // Title line
//                                         display.setTextSize(char_size - 1);
//                                         display.setTextColor(BLACK, WHITE);
//                                         display.setCursor(0, line_space * (n + 1));
//                                         display.print(Lines[n]);
//                                 }
//                         }
//                 }
//         }
//         else
//         {
//                 for (int n = 0; n < 4; n++)
//                 {
//                         if (strcmp(Lines[n], "") != 0)
//                         {
//                                 if (n == 1 || n == 3)
//                                 { // Clocks
//                                         display.setTextSize(char_size);
//                                         display.setTextColor(WHITE);
//                                         int strLength = strlen(Lines[n]);
//                                         display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * n - 3);
//                                         display.print(Lines[n]);
//                                 }
//                                 else
//                                 { // Title
//                                         display.setTextSize(char_size - 1);
//                                         display.setTextColor(BLACK, WHITE);
//                                         display.setCursor(0, line_space * n);
//                                         display.print(Lines[n]);
//                                 }
//                         }
//                 }
//         }
//         display.display();
// }

// void swapLines(char *line1, char *line2, char *line3, char *line4, byte num_lines = 2)
// {
//         char *txtLines[] = {line1, line2, line3, line4};
//         static byte line_shift = 0;
//         if (num_lines == 2)
//         {
//                 OLED_CenterTXT(2, txtLines[0 + line_shift], );
//         }

//         byte empty_lines = 0;
//         for (int a = 0; a < 4; a++)
//         {
//                 if (strcmp(txtLines[a], "") == 0)
//                 {
//                         empty_lines++;
//                 }
//         }
// }

// void OLEDlooper(int swapTime = 5000)
// {
//         iot.return_clock(timeStamp);
//         iot.return_date(dateStamp);

//         char DHTreading[20];
//         char DEGREE_SYMBOL = {167};
//         sprintf(DHTreading, "%.1fC %.0f%%", t, h);

//         // OLED_CenterTXT(2, timeStamp, DHTreading);
//         if (burstMSG() == 0)
//         {
//                 swapLines(timeStamp, dateStamp, DHTreading, "", SCREEN_HEIGHT / 16);
//                 if (swapLines_counter == 0)
//                 {
//                         swapLines_counter = millis();
//                 }
//                 if (millis() - swapLines_counter < swapTime)
//                 {
//                         OLED_CenterTXT(2, timeStamp, dateStamp);
//                 }
//                 else if (millis() - swapLines_counter >= swapTime && millis() - swapLines_counter <= 2 * swapTime)
//                 {
//                         OLED_CenterTXT(2, timeStamp, DHTreading);
//                 }
//                 else if (millis() - swapLines_counter > 2 * swapTime)
//                 {
//                         swapLines_counter = 0;
//                 }
//         }
// }
// #endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    

myScreens::myScreens(char *screen_type, int rows, int cols)
{
}

void myScreens::startLCD()
{
    LiquidCrystal_I2C lcd(0x27, _lcdColumns, 5);
    lcd.init();
    lcd.backlight();
}

// void edit_burstMSG(char *line1, char *line2, int duration_sec)
// {
//         sprintf(bmsg[0], "%s", line1);
//         sprintf(bmsg[1], "%s", line2);
//         bmsg_t = millis() + 1000 * duration_sec;
// }
// bool burstMSG()
// {
//         if (bmsg_t - (long)millis() >= 0)
//         {
//                 // OLED_CenterTXT(2, bmsg[0], bmsg[1]);
//                 return 1;
//         }
//         else
//         {
//                 return 0;
//         }
// }

// void screen_looper()
// {
//         iot.return_clock(timeStamp);
//         iot.return_date(dateStamp);

//         lcd.setCursor(0, 0);
//         lcd.print(timeStamp);
//         lcd.setCursor(lcdColumns / 2, 0);
//         lcd.print(dateStamp);

// #if USE_DHT30 || USE_TEMP_HUMID
//         char DHTreading[20];
//         sprintf(DHTreading, "Temp:%.1f%cC Hum:%.0f%%", t, 223, h);
//         lcd.setCursor((int)(lcdColumns - strlen(DHTreading)) / 2, 1);
//         lcd.print(DHTreading);
// #endif

// lcd.setCursor(0, 2);
// char line3 [25];
// char t1[30];
// char t2[2][30];
// lcd.print(" Shachar       Oz   ");
// for (int i=0; i<NUM_SWITCHES; i++){
//         if (TO[i]->remain()>0){
//                 TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), t2[i], t1);
//         }
//         else{
//                 sprintf(t2[i],"Off");
//         }
// }
// sprintf(line3,"%s %s",t2[0], t2[1]);
// lcd.setCursor((int)(20/2-strlen(t2[0]))/2, 3);
// lcd.print(t2[0]);
// lcd.setCursor(10+(int)(20/2-strlen(t2[0]))/2, 3);
// lcd.print(t2[1]);

// // delay(50);
// // lcd.clear();
// }
// #endif

// // ~~~~~~~~~~~~~~ IFTTT ~~~~~~~~~~~

// // ########################### END ADDITIONAL SERVICE ##########################

// void setup()
// {
// #if HARD_REBOOT
//         EEPROM.begin(1024);
//         check_hardReboot();
// #endif

//         startGPIOs();
//         quickPwrON();
//         startIOTservices();
//         startTO();

// #if USE_SENSOR
//         sensSW.start();
// #endif

// #if USE_NOTIFY_TELE
//         teleNotify.begin(telecmds);
// #endif

// #if USE_IR_REMOTE
//         start_IR();
// #endif
// #if USE_SCREEN
// #if USE_OLED
//         startOLED();
// #endif
// #if USE_LCD
//         startLCD();
// #endif
// #endif
// #if USE_TEMP_HUMID
//         startDHT();
// #endif

// #if USE_PIR_SESNOR
//         startPIR();
// #endif
// }

// void loop()
// {
//         iot.looper();

// #if USE_RESETKEEPER
//         if (checkrebootState == true)
//         {
//                 recoverReset();
//         }
// #endif

//         for (int i = 0; i < NUM_SWITCHES; i++)
//         {
//                 if (USE_INPUTS)
//                 {
//                         checkSwitch_Pressed(i, IS_MOMENTARY);
//                 }
//                 TO_looper(i);
//         }
//         // digitalWrite(indic_LEDpin, !relState[0]);

// #if USE_NOTIFY_TELE
//         teleNotify.looper();
// #endif

// #if USE_SENSOR
//         checkSensor_looper(0);
// #endif

// #if USE_IR_REMOTE
//         recvIRinputs(); // IR signals
// #endif
// #if USE_SCREEN
// #if USE_OLED
//         OLEDlooper();
// #endif
// #if USE_LCD
//         screen_looper();
// #endif
// #endif

// #if USE_TEMP_HUMID
//         getTH_reading();
// #endif
// #if USE_DHT30
//         DHT30_loop();
//         #endif

// #if USE_IFTTT
//         IFTT_lopper();
// #endif

// #if USE_PIR_SESNOR
//         PIRsensor0.looper();
// #endif
//         delay(50);
// }
