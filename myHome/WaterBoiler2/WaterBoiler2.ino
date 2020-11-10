#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1 D4
#define INPUT1 D6
#define indic_LEDpin D7

// ~~~~~~~~ state Vars ~~~~~~~~~~~
#define RelayOn HIGH
#define SwitchOn LOW
#define ledON HIGH
bool last_relState;

#include <myIOT.h>
#include "myIOT_settings.h"
#include "TO_settings.h"

// ~~~~~~~TimeOut Constants ~~~~~~
const int maxTO = 150;         //minutes to timeout even in ON state
const int timeIncrements = 15; //minutes each button press
const int deBounceInt = 50;
const int time_NOref_OLED = 5; // seconds to stop refresh OLED
int timeInc_counter = 0;       // counts number of presses to TO increments
unsigned long clock_noref = 0;

// ~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~~~~~~~
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET LED_BUILTIN
long swapLines_counter = 0;
long on_clock = 0;
char timeStamp[50];
char dateStamp[50];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void startOLED()
{
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
}
void OLED_CenterTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0, byte y_shift = 0)
{
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        display.setTextColor(WHITE);
        byte line_space = pow(2, (2 + char_size));

        for (int n = 0; n < 4; n++)
        {
                if (strcmp(Lines[n], "") != 0)
                {
                        int strLength = strlen(Lines[n]);
                        display.setCursor((ceil)((21 / char_size - strLength) / 2 * (128 / (21 / char_size))) + x_shift, line_space * n + y_shift);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void OLED_SideTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "")
{
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        byte line_space = pow(2, (2 + char_size));

        if (strcmp(line3, "") == 0 && strcmp(line4, "") == 0)
        { // for ON state only - 2rows
                for (int n = 0; n < 2; n++)
                {
                        if (strcmp(Lines[n], "") != 0)
                        {
                                if (n == 1)
                                { // Clock line
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * (n + 1) - 3);
                                        display.print(Lines[n]);
                                }
                                else
                                { // Title line
                                        display.setTextSize(char_size - 1);
                                        display.setTextColor(BLACK, WHITE);
                                        display.setCursor(0, line_space * (n + 1));
                                        display.print(Lines[n]);
                                }
                        }
                }
        }
        else
        {
                for (int n = 0; n < 4; n++)
                {
                        if (strcmp(Lines[n], "") != 0)
                        {
                                if (n == 1 || n == 3)
                                { // Clocks
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * n - 3);
                                        display.print(Lines[n]);
                                }
                                else
                                { // Title
                                        display.setTextSize(char_size - 1);
                                        display.setTextColor(BLACK, WHITE);
                                        display.setCursor(0, line_space * n);
                                        display.print(Lines[n]);
                                }
                        }
                }
        }
        display.display();
}

void sec2clock(int sec, char *text, char *output_text)
{
        int h = ((int)(sec) / (60 * 60));
        int m = ((int)(sec)-h * 60 * 60) / (60);
        int s = ((int)(sec)-h * 60 * 60 - m * 60);
        sprintf(output_text, "%s %01d:%02d:%02d", text, h, m, s);
}
void display_totalOnTime()
{
        char msg[150];
        int totalONtime = now() - TOswitch.TOswitch.getStart_to();
        sec2clock(totalONtime, "", msg);
        clock_noref = millis();
        OLED_CenterTXT(2, "ON time:", "", msg);
}
void display_ON_clock()
{
        char time_on_char[20];
        char time2Off_char[20];
        int timeON = now() - TOswitch.TOswitch.getStart_to();
        int timeLeft = TOswitch.TOswitch.remain();

        sec2clock(timeON, "", time_on_char);
        if (timeLeft == 0)
        {
                OLED_SideTXT(2, "On:", time_on_char);
        }
        else
        {
                sec2clock(timeLeft, "", time2Off_char);
                OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
        }
}
void display_OFF_clock()
{
        iot.return_clock(timeStamp);
        iot.return_date(dateStamp);

        int timeQoute = 5000;

        if (swapLines_counter == 0)
        {
                swapLines_counter = millis();
        }
        if (millis() - swapLines_counter < timeQoute)
        {
                OLED_CenterTXT(2, "", timeStamp, dateStamp, "");
        }
        else if (millis() - swapLines_counter >= timeQoute && millis() - swapLines_counter < 2 * timeQoute)
        {
                OLED_CenterTXT(2, timeStamp, "", "", dateStamp);
        }
        else if (millis() - swapLines_counter > 2 * timeQoute)
        {
                swapLines_counter = 0;
        }
}
void OLEDlooper()
{
        // if (clock_noref == 0)
        // { // freeze OLED display - to display
        if (digitalRead(RELAY1) == RelayOn)
        {
                display_ON_clock();
        }
        else
        { // OFF state - clock only
                display_OFF_clock();
        }

        // else
        // {
        //         if (millis() - clock_noref > time_NOref_OLED * 1000)
        //         { // time in millis without screen refresh
        //                 clock_noref = 0;
        //         }
        // }
}

void switchOff(char *txt)
{
        if (TOswitch.TOswitch.remain() > 0) // in TO mode
        {
                TOswitch.TOswitch.endNow();
        }
        else // ON but not in TO mode
        {
                display_totalOnTime();
                TOswitch.switchIt("txt", (float)0);
                TOswitch.TOswitch.updateStart(0);
        }
        timeInc_counter = 0;
}
void switchOn(char *txt)
{
        if (timeInc_counter == 0 && last_relState == !RelayOn)
        { // first press turns on - still not in TO mode
                TOswitch.switchIt(txt, (float)1);
                TOswitch.TOswitch.updateStart(now()); // register when started in FS
                timeInc_counter += 1;
        }
        // CASE of already on, and insde interval of time - to add timer Qouta
        else if (timeInc_counter < (maxTO / timeIncrements))
        {
                char msg[50];
                char tempstr[20];

                int newTO = TOswitch.TOswitch.remain() + 1 * timeIncrements * 60; // add a timeQoute to on going TO
                TOswitch.TOswitch.setNewTimeout(newTO, false);
                sec2clock((timeInc_counter)*timeIncrements * 60, "Button: Added Timeout: +", msg);
                timeInc_counter += 1; // Adding time Qouta
                sprintf(tempstr, "%s: %s", txt, msg);
                iot.pub_msg(tempstr);
        }
}
void press_cases(int &pressedTime, int &max_time_pressed)
{
        // CASE of it is first press and Relay was off - switch it ON, no timer.
        if (pressedTime >= max_time_pressed - 500)
        {
                switchOff("Button");
        }
        else
        {
                switchOn("Button");
        }
}
void checkSwitch_Pressed()
{
        long timeCouter = 0;
        int pressedTime = 0;
        int max_time_pressed = 1500;
        static bool still_pressed = false;

        if (digitalRead(INPUT1) == SwitchOn)
        {
                delay(deBounceInt);
                if (digitalRead(INPUT1) == SwitchOn && still_pressed == false)
                {
                        while (digitalRead(INPUT1) == SwitchOn) // count press duration
                        {
                                if (timeCouter == 0)
                                {
                                        timeCouter = millis();
                                }
                                if (millis() - timeCouter >= max_time_pressed)
                                {
                                        still_pressed = true;
                                        break;
                                }
                        }
                        pressedTime = millis() - timeCouter;
                        press_cases(pressedTime, max_time_pressed);
                }
        }
        else
        {
                still_pressed = false;
        }
}

void setup()
{
        TOswitch_init();
        startIOTservices();
        startOLED();
        startdTO();
        pinMode(INPUT1, INPUT);
}
void loop()
{
        iot.looper();
        OLEDlooper();
        TOswitch_looper();
        checkSwitch_Pressed();
        digitalWrite(indic_LEDpin, digitalRead(RELAY1));
        delay(100);
}
