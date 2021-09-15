#define BUTTON_PIN 5
#define BUTTON_PRESSED LOW
#define MAX_BUTTON_PRESSES 4

myOLED OLED;
uint8_t but_press_counter = 0;
void start_button()
{
        pinMode(BUTTON_PIN, INPUT);
}
void read_button()
{
        if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED)
        {
                delay(50);
                if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED)
                {
                        if (but_press_counter < MAX_BUTTON_PRESSES)
                        {
                                but_press_counter++;
                        }
                        else
                        {
                                but_press_counter = 0;
                        }
                }
        }
}
void startOLED()
{
        OLED.start();
        // OLED.CenterTXT("BootUp", "IPmonitoring");
}
void display_boot()
{
        char txt[4][12];
        char uptime[12];
        char upday[5];
        iot.convert_epoch2clock(iot.now(), WiFi_service.bootClk, uptime, upday);

        sprintf(txt[0], "BootUp");
        sprintf(txt[1], "%s", WiFi_service.libVer);
        iot.return_clock(txt[2]);
        iot.return_date(txt[3]);
        OLED.CenterTXT(txt[0], txt[1], txt[2], uptime);
}
void compose_OLEDtxt0(uint8_t &x, char a[], IPmonitoring &ipmon)
{
        sprintf(a, "%s %s", ipmon.nick, ipmon.isConnected ? "V" : "x");
        x++;
}
void display_OK()
{
        char txt[4][10];

        for (int n = 0; n < 4; n++)
        {
                sprintf(txt[n], "");
        }
        uint8_t inc = 0;

        if (check_all_services)
        {
                sprintf(txt[0], "All OK");
                iot.convert_epoch2clock(iot.now(), WiFi_service.bootClk, txt[3], txt[2]);
                // iot.return_clock(txt[2]);
                // iot.return_date(txt[3]);
        }
        else
        {
#if CHECK_MQTT
                compose_OLEDtxt0(inc, txt[inc], MQTT_service);
#endif
#if CHECK_WIFI
                compose_OLEDtxt0(inc, txt[inc], WiFi_service);

#endif
#if CHECK_INTERNET
                compose_OLEDtxt0(inc, txt[inc], Internet_service);
#endif
#if CHECK_HASS
                compose_OLEDtxt0(inc, txt[inc], HASS_service);
#endif
        }
        OLED.CenterTXT(txt[0], txt[1], txt[2], txt[3]);
}
void display_1()
{
}
void display_stats(uint8_t &i)
{
}
void displays_looper()
{
        static unsigned long disp_t = 0;
        const uint8_t Sec_boot_display = 25;
        if (millis() < Sec_boot_display * 1000)
        {
                display_boot();
        }
        else
        {
                if (but_press_counter == 0)
                {
                        if (check_all_services) // All Sevices are OK
                        {
                                display_OK();
                        }
                }
                else
                {
                }
        }
}
// #endif

#include <myDisplay.h>

// extern myIOT2 iot;
// extern unsigned long bootclk;
// extern bool mqttConnected;
// extern bool internetConnected;
// extern bool homeAssistantConnected;
// extern const byte PERIOD_0;
// extern const byte PERIOD_1;
// extern const byte PERIOD_2;
// extern const byte PERIOD_3;
// extern unsigned long period_disc_cumTime[4];
// extern const byte NUM_PERIODS;
// extern const float check_times[];
// extern int period_disconnects[4];
// extern int alertLevel[4];
// extern int getLOG_entries(unsigned long LOG[]);
// extern bool verifyLOG();

// extern int buttonPin;

// // ~~~~~~~~~~~~ OLED Display ~~~~~~~~~~~~~~~~
// myOLED OLED(64, 128);
// char dispLines[4][40];

// float get_alertLevel()
// {
//     float sum = 0;
//     const byte HIGH_SCORE = 2;
//     int Max = 0;

//     byte relation[] = {20, 40, 30, 10};
//     for (int i = 0; i < NUM_PERIODS; i++)
//     {
//         int a = alertLevel[i] * relation[i];
//         sum += alertLevel[i] * relation[i];
//         Max = max(Max, alertLevel[i]);
//     }
//     sum = 1.0 - (float)((0.01 * sum) / HIGH_SCORE);
//     return sum * 100;
// }
// void displays_0()
// {
//     /* Mains display */
//     char days[10];
//     char clk[20];

//     float alLevel = get_alertLevel();
//     sprintf(dispLines[1], "   %.0f%%", alLevel);
//     iot.convert_epoch2clock(now(), bootclk, clk, days);
//     sprintf(dispLines[3], "    %s", days);
//     sprintf(dispLines[2], "  %s", clk);
//     bool services = internetConnected && mqttConnected && homeAssistantConnected && WiFi.isConnected();

//     OLED.display.clearDisplay();

//     // First Row - Services status
//     OLED.display.setTextSize(1);
//     OLED.display.setCursor(0, 0);
//     OLED.display.setTextColor(BLACK, WHITE);
//     OLED.display.print("Services:");

//     OLED.display.setTextSize(2);
//     OLED.display.setTextColor(WHITE);
//     if (services)
//     {
//         OLED.display.print("    OK");
//     }
//     else
//     {
//         OLED.display.print("  Fail");
//     }

//     // Second Row - Connection Errors
//     OLED.display.setTextSize(1);
//     OLED.display.setCursor(0, 16);
//     OLED.display.setTextColor(BLACK, WHITE);
//     OLED.display.print("Status:");
//     OLED.display.setTextSize(2);
//     OLED.display.setTextColor(WHITE);
//     OLED.display.print(dispLines[1]);

//     // Third Row- uptime
//     OLED.display.setTextSize(1);
//     OLED.display.setCursor(0, 32);
//     OLED.display.setTextColor(BLACK, WHITE);
//     OLED.display.print("UPtime:");
//     OLED.display.setTextSize(2);
//     OLED.display.setTextColor(WHITE);
//     OLED.display.print(dispLines[3]);

//     // Forth Row
//     OLED.display.setCursor(5, 48);
//     OLED.display.print(dispLines[2]);

//     OLED.display.display();
// }
// void displays_3()
// {
//     /* Clock & IP */
//     char days[10];
//     char clk[20];

//     OLED.display.clearDisplay();

//     // First Row - Services status
//     OLED.display.setTextSize(1);
//     OLED.display.setCursor(0, 0);
//     OLED.display.setTextColor(BLACK, WHITE);
//     OLED.display.print("IP:");

//     OLED.display.setTextSize(1);
//     OLED.display.setTextColor(WHITE);
//     sprintf(dispLines[0], "    %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
//     OLED.display.print(dispLines[0]);

//     // First Row - Services status
//     OLED.display.setTextSize(1);
//     OLED.display.setCursor(0, 16);
//     OLED.display.setTextColor(BLACK, WHITE);
//     OLED.display.print("LOG:");

//     OLED.display.setTextSize(2);
//     OLED.display.setTextColor(WHITE);

//     if (verifyLOG())
//     {
//         int x = getLOG_entries(connLOG);
//         sprintf(dispLines[1], " OK #%d", x);
//         OLED.display.print(dispLines[1]);
//     }
//     else
//     {
//         OLED.display.print("Error");
//     }

//     OLED.display.display();
// }

// void display_services()
// {
//     /* Services Status */
//     OLED.char_size = 2;
//     sprintf(dispLines[0], "network:%s", internetConnected ? "OK" : "X");
//     sprintf(dispLines[1], "MQTT: %s", mqttConnected ? "  OK" : "X");
//     sprintf(dispLines[2], "HASS: %s", homeAssistantConnected ? "  OK" : "X");
//     sprintf(dispLines[3], "WiFi: %s", WiFi.isConnected() ? "  OK" : "X");
//     OLED.CenterTXT(dispLines[0], dispLines[1], dispLines[2], dispLines[3]);
// }

// void display_err()
// {
//     if (!WiFi.isConnected())
//     {
//         OLED.char_size = 2;
//         OLED.CenterTXT("WiFi", "NOT", "Connected");
//     }
//     else if (!internetConnected)
//     {
//         OLED.char_size = 2;
//         OLED.CenterTXT("", "NO", "Internet", "Connection");
//     }
// }

// void display_periodic_reports(byte i)
// {
//     /* 1st Time Period */
//     char clk[20];
//     char days[5];
//     iot.convert_epoch2clock(period_disc_cumTime[i], 0, clk, days);
//     sprintf(dispLines[1], "%s", days);
//     sprintf(dispLines[2], "%s", clk);

//     sprintf(dispLines[3], "A:%d D:%d", alertLevel[i], period_disconnects[i]);

//     OLED.char_size = 2;
//     sprintf(dispLines[0], "~%.0f_hrs~", check_times[i]);
//     OLED.CenterTXT(dispLines[0], dispLines[1], dispLines[2], dispLines[3]);
// }

// byte buttonCounter(byte cycle)
// {
//     static byte count = 0;
//     static unsigned long reset_counter = 0;
//     const byte _time2zero = 10; //sec
//     if (digitalRead(buttonPin) == LOW)
//     {
//         delay(50);
//         if (digitalRead(buttonPin) == LOW)
//         {
//             reset_counter = millis();
//             if (count <= cycle - 1)
//             {
//                 count++;
//             }
//             else
//             {
//                 count = 0;
//             }
//         }
//     }
//     if (reset_counter != 0)
//     {
//         if (reset_counter + _time2zero * 1000UL <= millis())
//         {
//             count = 0;
//             reset_counter = 0;
//         }
//     }
//     return count;
// }

// void displays_looper()
// {
//     int del = 50;
//     byte a;
//     if (!internetConnected || !WiFi.isConnected() || a != 0)
//     {
//         display_err();
//     }
//     else
//     {
//         a = buttonCounter(6);

//         switch (a)
//         {
//         case 0:
//             displays_0();
//             delay(del);
//             break;
//         case 1:
//             display_services();
//             delay(del);
//             break;
//         case 2:
//             display_periodic_reports(0);
//             delay(del);
//             break;
//         case 3:
//             display_periodic_reports(1);
//             delay(del);
//             break;
//         case 4:
//             display_periodic_reports(2);
//             delay(del);
//             break;
//         case 5:
//             display_periodic_reports(3);
//             delay(del);
//             break;
//         case 6:
//             displays_3();
//             delay(del);
//             break;
//         default:
//             displays_0();
//             delay(del);

//             break;
//         }
//     }
// }
