#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "constants.h"
#include <myIOT2.h>
#include "myIOT_settings.h"
#include "TO_settings.h"

int timeInc_counter = 0; // counts number of presses to TO increments
unsigned long clock_noref = 0;
unsigned long onclk = 0;

// ~~~~~~~~~~~~~~ OLED display ~~~~~~~~~~~~~~~~~
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

void sec2clock(int sec, char *output_text)
{
        int h = ((int)(sec) / (60 * 60));
        int m = ((int)(sec)-h * 60 * 60) / (60);
        int s = ((int)(sec)-h * 60 * 60 - m * 60);
        sprintf(output_text, "%01d:%02d:%02d", h, m, s);
}
void display_totalOnTime()
{
        char msg[50];
        sec2clock((int)(iot.now() - onclk), msg);
        clock_noref = millis(); // start clock to Frozen msg
        OLED_CenterTXT(2, "Total", "ON time:", msg);
}
void display_ON_clock()
{
        char time_on_char[20];
        char time2Off_char[20];

        if (onclk == 0)
        {
                onclk = TOswitch.onClk();
        }

        sec2clock((int)(iot.now() - onclk), time_on_char);
        if ((int)(iot.now() - onclk) == 0)
        {
                OLED_SideTXT(2, "On:", time_on_char);
        }
        else
        {
                sec2clock(TOswitch.remTime(), time2Off_char);
                OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
        }
}
void display_OFF_clock()
{
        char timeStamp[50];
        char dateStamp[50];
        static long swapLines_counter = 0;

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
        if (clock_noref == 0) // OLED is not Frozen with a static msg
        {
                if (digitalRead(RELAY1) == RelayOn)
                {
                        display_ON_clock();
                }
                else
                { // OFF state - clock only
                        display_OFF_clock();
                }
        }

        else /* Display Frozen msg*/
        {
                if (millis() - clock_noref > time_NOref_OLED * 1000)
                {                        // time in millis without screen refresh
                        clock_noref = 0; /* stop Frozen display */
                }
        }
}

// ~~~~~~~~~~~~~~ Button Press ~~~~~~~~~~~~~~~~~
void press_cases(int &pressedTime, const int &max_time_pressed)
{
        if (pressedTime > max_time_pressed - 500) /* Long press - turning off */
        {
                TOswitch.finish_TO(0);
                timeInc_counter = 0;
                onclk = 0;
        }
        else
        {
                if (TOswitch.remTime() == 0) /* Turning ON, minimal timer */
                {
                        TOswitch.start_TO(timeIncrements, 0, true);
                        onclk = TOswitch.onClk();
                }
                else /* Add time to timer*/
                {
                        timeInc_counter++;
                        TOswitch.start_TO(timeIncrements * 60 + TOswitch.remTime(), 0, false);
                        TOswitch.updateEndClk(TOswitch.TO_duration, TOswitch.onClk());
                }
        }
}
void checkSwitch_Pressed()
{
        long timeCouter = 0;
        int pressedTime = 0;
        const int max_time_pressed = 1500;
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
                                else if (millis() - timeCouter >= max_time_pressed)
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
        digitalWrite(indic_LEDpin, digitalRead(RELAY1));
}

void startGPIO()
{
        pinMode(INPUT1, INPUT_PULLUP); // not defined in mySwitch, but here. Locally.
        pinMode(indic_LEDpin, OUTPUT);
        pinMode(RELAY1, OUTPUT);
        digitalWrite(RELAY1, !RelayOn);
}

void setup()
{
        startGPIO();
        startIOTservices();
        TOswitch_init();
        startOLED();
}
void loop()
{
        iot.looper();
        OLEDlooper();
        checkSwitch_Pressed();
        TOswitch.looper();
        delay(100);
}
