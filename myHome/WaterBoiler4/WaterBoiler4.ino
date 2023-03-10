#include <myIOT2.h>
#include <smartSwitch.h>

myIOT2 iot;
smartSwitch SWitch;

const char *VEr = "theDude_v4";

#include "myIOT_settings.h"

#define RELAY1 D5
#define INPUT1 D6
#define indic_LEDpin D7
#define START_DURATION_TIMEOUT 30 // minutes
#define ADDITIONAL_TIME_PRESS 15  // minutes

struct saveOper
{
        unsigned long day_accum = 0;
        time_t date = 0;
};
saveOper saver;

#define USE_OLED_SCREEN true
#if USE_OLED_SCREEN
#include "oled.h"
#endif

void update_logs(unsigned long onTime)
{
        time_t t = iot.now();
        struct tm *tm = localtime(&t);
        uint8_t now_month = tm->tm_mon;
        uint8_t now_day = tm->tm_mday;

        struct tm *tm_saved = localtime(&saver.date);
        uint8_t save_month = tm_saved->tm_mon;
        uint8_t save_day = tm_saved->tm_mday;

        if (now_month == save_month && now_day == save_day)
        {
                saver.day_accum += onTime;
        }
        else // if (saver.date == 0) or any other situation
        {
                saver.date = t;
                saver.day_accum = onTime;
        }
}
void init_smartSwitch()
{
        SWitch.useDebug = false;
        SWitch.OUTPUT_ON = HIGH;
        SWitch.BUTTON_PRESSED = LOW;

        SWitch.set_id(1);
        SWitch.set_timeout(START_DURATION_TIMEOUT);
        SWitch.set_name("boiler");
        SWitch.set_input(INPUT1, MULTI_PRESS_BUTTON);
        SWitch.set_output(RELAY1);
        SWitch.set_indiction(indic_LEDpin, HIGH);
}
void smartSwitch_loop()
{
        if (SWitch.loop())
        {
                char clk[15];
                char msg[100];
                const char *states[] = {"Off", "On"};
                const char *REASONS_OPER[] = {"Button", "Timeout", "MQTT"};

                if (SWitch.telemtryMSG.state == 0)
                {
                        iot.convert_epoch2clock(SWitch.get_elapsed() / 1000, 0, clk);
                        clock_noref = millis();
                        sprintf(msg, "[%s]: turned [%s], ON time [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                states[SWitch.telemtryMSG.state], clk);

                        update_logs(SWitch.get_elapsed() / 1000);
                }
                else if (SWitch.telemtryMSG.state == 1)
                {
                        if (SWitch.telemtryMSG.pressCount > 1)
                        {
                                const int add_time = ADDITIONAL_TIME_PRESS;
                                SWitch.set_additional_timeout(add_time, SWitch.telemtryMSG.reason);
                                iot.convert_epoch2clock(SWitch.get_timeout() / 1000, 0, clk);
                                sprintf(msg, "[%s]: added [%d] minutes, updated timeout [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                        add_time, clk);
                        }
                        else
                        {
                                iot.convert_epoch2clock(SWitch.get_timeout() / 1000, 0, clk);
                                sprintf(msg, "[%s]: turned [%s], timeout [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                        states[SWitch.telemtryMSG.state], clk);
                        }
                }
                iot.pub_msg(msg);
                SWitch.clear_newMSG();
        }
}

void setup()
{
        startIOTservices();
#if USE_OLED_SCREEN
        startOLED();
#endif
        init_smartSwitch();
}
void loop()
{
        iot.looper();
#if USE_OLED_SCREEN
        OLED_display_looper();
#endif
        smartSwitch_loop();
}
