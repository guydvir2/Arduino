#include <myIOT2.h>>
#include <FastLED.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.3"
#define USE_IR_REMOTE false
#define COLOR 1
#define LED_DELAY 2        // ms
#define BRIGHTNESS 5       // [0,100]
#define LED_DIRECTION true // [0,1]
#define MAX_BRIGHT 100
#define MIN_BRIGHT 5
#define JUMP_BRIGHT 15
#define IR_SENSOR_PIN D5

//~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  ~~~~~~ LEDS ~~~~~~~~~~~~
#define NUM_LEDS 150
#define LED_DATA_PIN D4

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "kidsColorLEDs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT2 iot;


//  ~~~~~ LEDS ~~~~~
CRGB leds[NUM_LEDS];
byte ledBrightness = MAX_BRIGHT;
byte ledColor = COLOR;
bool Cylon_flag = false;

//~~~~~~~~~~~~~~~~~~~~LEDS defs ~~~~~~~~~~~~~~~~~~~~~

/*
   Color List:
   https://github.com/FastLED/FastLED/wiki/Pixel-reference
 */

CRGB colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x008000, 0x0000FF,
                 0xF0F8FF, 0x9966CC,0xFAEBD7, 0x7FFF00,0x8B0000,
                 0x696969,0xFF1493,0x90EE90,0x87CEEB,0xFF6347};

const char *color_names[] = {"Black", "White", "Red", "Green", "Blue",
                             "AliceBlue","Amethyst","AntiqueWhite","Chartreuse","DarkRed"
                             "DimGray","DeepPink","LightGreen","SkyBlue","Tomato"};

const int tot_colors = int(sizeof(colors) / sizeof(colors[0]));

// #############################################################################

void startIOTservices()
{
        iot.useSerial = false;
        iot.useWDT = true;
        iot.useOTA = true;
        iot.useResetKeeper = true;
        iot.useextTopic = false;
        iot.useDebug = false;
        iot.debug_level = 0;
        iot.useNetworkReset = true;
        iot.noNetwork_reset = 10;
        iot.useBootClockLog = false;
        iot.useAltermqttServer = false;
        iot.ignore_boot_msg = false;
        iot.deviceTopic = DEVICE_TOPIC;
        iot.prefixTopic = MQTT_PREFIX;
        iot.addGroupTopic = MQTT_GROUP;
}


// ~~~~~~ LED Operations ~~~~~~~~~~
void turn_leds_off()
{

        for (int i = 0; i < NUM_LEDS; i++)
        {
                leds[i] = colors[0];
                delay(LED_DELAY);
                FastLED.show();
        }

        if (Cylon_flag)
        {
                Cylon_flag = false;
        }
}
void turn_leds_on(int col_indx = COLOR, int bright_1 = BRIGHTNESS, int del_1 = LED_DELAY, bool dir_1 = LED_DIRECTION)
{

        ledBrightness = bright_1;
        if (col_indx <= tot_colors && bright_1 <= MAX_BRIGHT && del_1 <= 1000 && dir_1 <= 1)
        {
                FastLED.setBrightness((bright_1 * 255 / 100));
                if (dir_1 == true)
                { // start to end
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                                leds[i] = colors[col_indx];
                                // delay(del_1);
                        }
                }
                else
                { // end to start
                        for (int i = NUM_LEDS - 1; i >= 0; i = i - 1)
                        {
                                leds[i] = colors[col_indx];
                                // FastLED.show();
                                // delay(del_1);
                        }
                }
        }
        FastLED.show();
}
void set_bright(byte val)
{
        char msg[50];

        ledBrightness = val;
        turn_leds_on(ledColor, ledBrightness, 0);

        sprintf(msg, "Brightness: Changed to [%d], range:[%d/%d]", val, MIN_BRIGHT, MAX_BRIGHT);
        iot.pub_msg(msg);
}
void set_color(byte col_i)
{
        char msg[50];

        if (col_i < tot_colors - 1 && col_i >= 0)
        {
                ledColor = col_i;
                // turn_leds_off();
                turn_leds_on(ledColor, ledBrightness);

                sprintf(msg, "Color: Changed to [%s] out of [%d] colors Available", color_names[col_i], tot_colors);
                iot.pub_msg(msg);
        }
}
void chng_brightness(int val)
{
        if (ledBrightness + val <= MAX_BRIGHT && ledBrightness + val >= MIN_BRIGHT)
        {
                ledBrightness += val;
                set_bright(ledBrightness);
        }
        else if (ledBrightness + val > MAX_BRIGHT)
        {
                ledBrightness = MAX_BRIGHT;
                set_bright(ledBrightness);
        }
        else if (ledBrightness + val < MIN_BRIGHT)
        {
                ledBrightness = MIN_BRIGHT;
                set_bright(ledBrightness);
        }
}
void chng_color(int col_i)
{
        if (ledColor + col_i < tot_colors - 1 && ledColor + col_i >= 0)
        {
                ledColor += col_i;
                set_color(ledColor);
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

void start_LEDS()
{
        LEDS.addLeds<WS2812, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
}
void fadeall()
{
        for (int i = 0; i < NUM_LEDS; i++)
        {
                leds[i].nscale8(250);
        }
}
void LEDS_looper()
{
        // static uint8_t hue = 0;
        uint8_t hue = 0;

        for (int i = 0; i < NUM_LEDS; i++)
        {
                leds[i] = CHSV(hue++, 255, 255);
                FastLED.show();
                // leds[i] = CRGB::Black;
                fadeall();
                delay(10);
        }

        for (int i = (NUM_LEDS)-1; i >= 0; i--)
        {
                leds[i] = CHSV(hue++, 255, 255);
                FastLED.show();
                fadeall();
                delay(10);
        }
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];

        if (strcmp(incoming_msg, "status") == 0)
        {
                if (Cylon_flag == false)
                {
                        sprintf(msg, "Status: Color [%s], Brightness [%d]", color_names[ledColor], ledBrightness);
                        iot.pub_msg(msg);
                }
                else
                {
                        sprintf(msg, "Status: Color [%s], Brightness [%s]", "Cylon Palette", ledBrightness);
                        iot.pub_msg(msg);
                }
        }
        // else if (strcmp(incoming_msg, "ver") == 0)
        // {
        //         sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "help") == 0)
        // {
        //         sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
        //         iot.pub_msg(msg);
        //         sprintf(msg, "Help: Commands #2 - [on, off, bright, color]");
        //         iot.pub_msg(msg);
        // }
        else if (strcmp(incoming_msg, "off") == 0)
        {
                turn_leds_off();
                sprintf(msg, "MQTT: Turn [OFF]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0)
        {
                turn_leds_on();
                sprintf(msg, "MQTT: Turn [ON]");
                iot.pub_msg(msg);
        }
        else
        {
                iot.inline_read(incoming_msg);

                if (strcmp(iot.inline_param[0], "bright") == 0)
                {
                        set_bright(atoi(iot.inline_param[1]));
                }
                else if (strcmp(iot.inline_param[0], "color") == 0)
                {
                        set_color(atoi(iot.inline_param[1]));
                }
        }
}


void setup()
{
        startIOTservices();
        start_LEDS();
        turn_leds_on();
}
void loop()
{
        iot.looper();
        if (Cylon_flag)
        {
                LEDS_looper();
        }

        delay(100);
}
