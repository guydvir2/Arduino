#include <myIOT.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Arduino.h>

// ********** Names + Strings  ***********
// #define Telegram_Nick "test1"      // belongs to TELEGRAM
// #define sensor_notification_msg "" // belongs to SENSOR

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "WaterBoiler2"
#define MQTT_PREFIX  "TESTS"
#define MQTT_GROUP   ""


// ********** Sketch Services  ***********
#define VER "Wemos_5.8"
#define USE_DAILY_TO  true
#define HARD_REBOOT   false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true // Serial Monitor
#define USE_WDT          true  // watchDog resets
#define USE_OTA          true  // OTA updates
#define USE_RESETKEEPER  true // detect quick reboot and real reboots
#define USE_FAILNTP      true  // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      1*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins
#define ON_AT_BOOT       false // true only for switches that powers up with device.

const int START_dailyTO[] = {18,0,0};
const int END_dailyTO[]   = {18,30,0};

int TIMEOUTS[2]  = {TIMEOUT_SW0, TIMEOUT_SW1};
timeOUT timeOut_SW0("SW0", TIMEOUTS[0]);
#if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1", TIMEOUTS[1]);
timeOUT *TO[] = {&timeOut_SW0, &timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[] = {&timeOut_SW0};
#endif
char *clockAlias = "Daily TimeOut";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1          D5
#define RELAY2          D2
#define INPUT1          D6
#define INPUT2          D5
#define indic_LEDpin    D7

byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define ledON            HIGH

bool relState[NUM_SWITCHES];
bool last_relState[NUM_SWITCHES];
int rebootState         = 0;
bool checkrebootState   = true;
bool boot_overide[]     = {true, true};


// #################################  END CORE #################################



//  ######################### ADDITIONAL SERVICES ##############################

// ~~~~~ Use Reset Counter for hardReboot ~~~~
struct eeproms_storage {
        byte jump;
        byte val;
        byte val_cell;
        byte wcount;
        byte wcount_cell;
        bool hBoot;
};

eeproms_storage hReset_eeprom;


// ~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~~~~~~~
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET    LED_BUILTIN
long swapLines_counter = 0;
char timeStamp [50];
char dateStamp [50];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// TimeOut Constants
int maxTO                    = 150; //minutes to timeout even in ON state
int timeIncrements           = 15; //minutes each button press
int timeInc_counter          = 0; // counts number of presses to TO increments
int delayBetweenPress        = 500; // consequtive presses to reset
int deBounceInt              = 50;
int time_NOref_OLED          = 10; // seconds to stop refresh OLED
unsigned long clock_noref    = 0;
unsigned long pressTO_input1 = 0; // TimeOUT for next press
unsigned long startTime      = 0;
unsigned long endTime        = 0;

// #############################################################################



//  ############################## STRART CORE #################################

void switchIt (char *txt1, int sw_num, bool state, char *txt2 = "", bool show_timeout = true) {
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word = {"Turned"};

        if (digitalRead(relays[sw_num]) != state || boot_overide[sw_num] == true) {
                digitalWrite(relays[sw_num], state);
                TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(), now(), msg1, msg2);
                if (boot_overide[sw_num] == true) {
                        if(iot.mqtt_detect_reset == 1 || TO[sw_num]->remain() > 0) { //BOOT TIME ONLY for after quick boot
                                word = {"Resume"};
                                boot_overide[sw_num] = false;
                        }
                }
                sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
                if (state == 1 && show_timeout) {
                        sprintf(msg2, "timeLeft[%s]", msg1);
                        strcat(msg, msg2);
                }

                iot.pub_msg(msg);

                sprintf(states, "");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", digitalRead(relays[i]) ? "ON" : "OFF");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
        }
}
void checkSwitch_Pressed(byte i) {
        char tempstr[20];
        char msg[50];
        if (digitalRead(inputs[i]) == SwitchOn) {
                delay(deBounceInt);
                // CASE #1 : Button is pressed. Delay creates a delay when buttons is pressed constantly
                if (digitalRead(inputs[i]) == SwitchOn && millis() - pressTO_input1 > delayBetweenPress) {
                        // CASE of it is first press and Relay was off - switch it ON, no timer.
                        if ( timeInc_counter == 0 && last_relState[i] == !RelayOn ) { // first press turns on
                                switchIt("Button", i, 1,"", false);
                                TO[i]->updateStart(now());
                                timeInc_counter += 1;
                        }
                        // CASE of already on, and insde interval of time - to add timer Qouta
                        else if (timeInc_counter < (maxTO / timeIncrements) && (millis() - pressTO_input1) < 2500 ) { // additional presses update timer countdown
                                int newTO = timeInc_counter * timeIncrements*60;
                                TO[i]->setNewTimeout(newTO, false);
                                sec2clock((timeInc_counter) * timeIncrements * 60, "Added Timeout: +", msg);
                                timeInc_counter += 1; // Adding time Qouta
                                sprintf(tempstr,"Button: Switch[#%d] %s", i, msg);
                                iot.pub_msg(tempstr);
                        }
                        // CASE of time is begger that time out-  sets it OFF
                        else if (timeInc_counter >= (maxTO / timeIncrements) || (millis() - pressTO_input1) > 2500) { // Turn OFF
                                if(TO[i]->remain()>0) {
                                        TO[i]->endNow();
                                }
                                else{
                                        display_totalOnTime();
                                        switchIt("Button", i, 0, "", false);
                                        TO[i]->updateStart(0);
                                }
                                timeInc_counter = 0;
                        }
                        pressTO_input1 = millis();
                }
        }
}
void startIOTservices() {
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);

                relState [i] = 0;
                last_relState [i] = 0;
        }
        pinMode(indic_LEDpin, OUTPUT);
}

// ~~~~~~ TimeOuts ~~~~~~~~~
void startTO(){
        for (int i=0; i<NUM_SWITCHES; i++) {
                TO[i]->begin(ON_AT_BOOT);
                if (USE_DAILY_TO) {
                        start_dailyTO(i);
                }
        }
}
void start_dailyTO(byte i){

        memcpy(TO[i]->dailyTO.on,START_dailyTO, sizeof(START_dailyTO));
        memcpy(TO[i]->dailyTO.off,END_dailyTO, sizeof(END_dailyTO));
        TO[i]->dailyTO.flag = USE_DAILY_TO;
        TO[i]->check_dailyTO_inFlash(TO[i]->dailyTO,i);
}
void TO_looper(byte i) {

        if (iot.mqtt_detect_reset != 2) {
                relState[i] = TO[i]->looper();
                if (relState[i] != last_relState[i]) { // change state (ON <-->OFF)
                        if (relState[i]==0) { // when TO ends
                                display_totalOnTime();
                                TO[i]->updateStart(0);
                                timeInc_counter = 0;
                        }
                        else{ // when TO starts
                                TO[i]->updateStart(now());
                        }
                        switchIt("TimeOut", i, relState[i]);
                }
                last_relState[i] = relState[i];
        }
}

// ~~~~ MQTT Commands ~~~~~
void addiotnalMQTT(char *income_msg) {
        char msg_MQTT[150];
        char msg2[20];

        if      (strcmp(income_msg, "status") == 0) {
                giveStatus(msg_MQTT);
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "ver") == 0 ) {
                sprintf(msg_MQTT, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "ver #2: DailyTO[%d], ON_AT_BOOT[%d], HardReboot[%d]", USE_DAILY_TO, ON_AT_BOOT, HARD_REBOOT);
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "help") == 0) {
                sprintf(msg_MQTT, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #2 - [remain, restartTO, timeout(x), endTO, updateTO(x), restoreTO, statusTO]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "flash") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(income_msg, "format") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.format();
        }
        else if (strcmp(income_msg, "all_off") == 0 ) {
                all_off("MQTT");

        }
        else {
                int num_parameters = iot.inline_read(income_msg);

                if (strcmp(iot.inline_param[1], "on") == 0 ) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false);
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false);
                }
                else if (strcmp(iot.inline_param[1], "timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg_MQTT);
                        sprintf(msg_MQTT, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg_MQTT, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg_MQTT);
                        delay(1000);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut update");
                }
                else if (strcmp(iot.inline_param[1], "remain") == 0) {
                        if (TO[atoi(iot.inline_param[0])]->remain()>0) {
                                TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg_MQTT);
                                sprintf(msg_MQTT, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
                                iot.pub_msg(msg_MQTT);
                        }
                }
                else if (strcmp(iot.inline_param[1], "restartTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg_MQTT, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "statusTO") == 0) {
                        sprintf(msg_MQTT, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
                                "TimeOut", atoi(iot.inline_param[0]),
                                TIMEOUTS[atoi(iot.inline_param[0])],
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "endTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->endNow();
                        sprintf(msg_MQTT, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "restoreTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restore_to();
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg_MQTT, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
                        iot.pub_msg(msg_MQTT);
                        iot.notifyOffline();
                        iot.sendReset("Restore");
                }
                else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0], TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2]);

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0], TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2]);

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0) {
                        sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
                                clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF",
                                TO[atoi(iot.inline_param[0])]->dailyTO.useFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg_MQTT);

                        Serial.println("IN STATUS_Daily LOOP");
                }
                else if (strcmp(iot.inline_param[1], "restart_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_dailyTO(TO[atoi(iot.inline_param[0])]->dailyTO);
                        sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }

                else{
                        if (strcmp(income_msg,"offline")!=0 && strcmp(income_msg,"online")!=0) {
                                sprintf(msg_MQTT,"Unrecognized Command: [%s]", income_msg);
                                iot.pub_err(msg_MQTT);
                        }
                }
                for (int n=0; n<=num_parameters -1; n++) {
                        sprintf(iot.inline_param[n],"");
                }
        }
}
void giveStatus(char *outputmsg){
        char t1 [50];
        char t2 [50];
        char t3 [50];

        sprintf(t3, "Status: ");
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->remain() > 0) {
                        TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), t2, t1);
                        sprintf(t1, "timeLeft[%s]", t2);
                }
                else {
                        sprintf(t1, "");
                }
                sprintf(t2, "Switch[#%d] [%s] %s ", i, digitalRead(relays[i]) ? "ON" : "OFF", t1);
                strcat(t3, t2);
        }
        sprintf(outputmsg,"%s",t3);
}
void all_off(char *from){
        char t[50];
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->remain() > 0 && relays[i] == RelayOn) {
                        TO[i]->endNow();
                }
                else if (TO[i]->remain() == 0 && relays[i] == RelayOn) {
                        switchIt(from, i, false,"", false);
                }
                else if (TO[i]->remain() > 0 && relays[i] != RelayOn) {
                        TO[i]->endNow();
                }
        }
        sprintf(t,"All OFF: Received from %s",from);
        iot.pub_msg(t);
}
//  ######################### ADDITIONAL SERVICES ##############################

// ~~~~ OLED ~~~~~~~
void startOLED() {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
}
void OLED_CenterTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0,  byte y_shift = 0) {
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        display.setTextColor(WHITE);
        byte line_space = pow(2, (2 + char_size));

        for (int n = 0; n < 4; n++) {
                if (strcmp(Lines[n], "") != 0) {
                        int strLength = strlen(Lines[n]);
                        display.setCursor((ceil)((21 / char_size - strLength) / 2 * (128 / (21 / char_size))) + x_shift,  line_space * n + y_shift);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void OLED_SideTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "") {
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        byte line_space = pow(2, (2 + char_size));


        if (strcmp(line3, "") == 0 && strcmp(line4, "") == 0) { // for ON state only - 2rows
                for (int n = 0; n < 2; n++) {
                        if (strcmp(Lines[n], "") != 0) {
                                if (n == 1) { // Clock line
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))),  line_space * (n + 1) - 3);
                                        display.print(Lines[n]);
                                }
                                else { // Title line
                                        display.setTextSize(char_size - 1);
                                        display.setTextColor(BLACK, WHITE);
                                        display.setCursor(0, line_space * (n + 1));
                                        display.print(Lines[n]);
                                }
                        }
                }

        }
        else {
                for (int n = 0; n < 4; n++) {
                        if (strcmp(Lines[n], "") != 0) {
                                if (n == 1 || n == 3) { // Clocks
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))),  line_space * n - 3);
                                        display.print(Lines[n]);
                                }
                                else { // Title
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
void OLEDlooper() {
        char time_on_char[20];
        char time2Off_char[20];

        if( clock_noref == 0) { // freeze OLED display

                if (digitalRead(relays[0]) == RelayOn ) {
                        int timeON = now() - TO[0]->getStart_to();
                        int timeLeft = TO[0]->remain();

                        sec2clock(timeON, "", time_on_char);
                        // if ( timeInc_counter == 1 ) { // ~~~~ON, no timer ~~~~~~~
                        if (timeLeft == 0) {
                                OLED_SideTXT(2, "On:", time_on_char);
                        }
                        // else if ( timeInc_counter > 1 || relState[0] == 1 ) { /// ON + Timer or DailyTO
                        else{
                                sec2clock(timeLeft, "", time2Off_char);
                                OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
                        }
                }
                else { // OFF state - clock only
                        iot.return_clock(timeStamp);
                        iot.return_date(dateStamp);
                        char *names[] = {"Guy","Anna","Shachar", "Oz", "Alex", "Dvir","*","**","***",":)"};
                        int name_amount = sizeof(names)/sizeof(char *);

                        int timeQoute = 5000;

                        if (swapLines_counter == 0) {
                                swapLines_counter = millis();
                        }
                        if (millis() - swapLines_counter < timeQoute) {
                                OLED_CenterTXT(2, names[random(0, name_amount)], timeStamp, dateStamp,names[random(0, name_amount)]);
                        }
                        else if (millis() - swapLines_counter >= timeQoute && millis() - swapLines_counter < 2 * timeQoute)
                        {
                                OLED_CenterTXT(2, timeStamp, names[random(0, name_amount)], names[random(0, name_amount)], dateStamp);
                        }
                        else if (millis() - swapLines_counter > 2 * timeQoute) {
                                swapLines_counter = 0;
                        }
                }
        }
        else{
                if (millis() - clock_noref > time_NOref_OLED*1000) { // time in millis
                        clock_noref = 0;
                }
        }
}
// ~~~~~~~~~~~~~~~

// ~~~~ string creation ~~~~~~
void sec2clock(int sec, char* text, char* output_text) {
        int h = ((int)(sec) / ( 60 * 60));
        int m = ((int)(sec) - h * 60 * 60) / ( 60);
        int s = ((int)(sec) - h * 60 * 60 - m * 60);
        sprintf(output_text, "%s %01d:%02d:%02d", text, h, m, s);
}
void display_totalOnTime(){
        char msg[150];
        int totalONtime = now() - TO[0]->getStart_to();
        sec2clock(totalONtime, "", msg);
        clock_noref = millis();
        OLED_CenterTXT(2,"ON time:","",msg);
}


// ~~~~~ BOOT ASSIST SERVICES ~~~~~~~~~
#if HARD_REBOOT
void check_hardReboot(byte i = 1, byte threshold = 2) {
        // hReset_eeprom.jump = EEPROM.read(0);
        hReset_eeprom.val_cell    = hReset_eeprom.jump + i;
        hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);

        if (hReset_eeprom.val < threshold) {
                EEPROM.write(hReset_eeprom.val_cell, hReset_eeprom.val + 1);
                EEPROM.commit();
                hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);
                hReset_eeprom.hBoot = false;
        }
        else {
                hReset_eeprom.hBoot = true;
        }
}
#endif

void quickPwrON() {
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */

        /*
         # conditions in for loop:
           1) Has more time to go in TO
           2) ON_AT_BOOT defines to be ON at bootTime
           3) eeprom Reset counter forces to be ON_AT_BOOT
         */

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->endTO_inFlash || ON_AT_BOOT || hReset_eeprom.hBoot) {
                        digitalWrite(relays[i], RelayOn);
                }
                else {
                        digitalWrite(relays[i], !RelayOn);
                }
        }
}
void recoverReset() {
        /*
           Using KeepAlive Service.
           This function determines if boot up caused be a regular PowerOn or caused
           by a power glich.
           The criteria is whther KeepAlive value was changed from
           "online" to "offline".
         */
        // Wait for indication if it was false reset(1) or
        char mqttmsg[30];
        rebootState = iot.mqtt_detect_reset;

        if (rebootState != 2) { // before getting online/offline MQTT state
                checkrebootState = false;
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (hReset_eeprom.hBoot && HARD_REBOOT) { // using HardReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> ForcedBoot. Restarting TimeOUT");
                        }
                        else if (rebootState == 0 && ON_AT_BOOT == true) {  // PowerOn - not a quickReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> NormalBoot & On-at-Boot. Restarting TimeOUT");
                        }
                        else if (TO[i]->looper() == 0) { // was not during TO
                                if (rebootState == 1) {
                                        iot.pub_err("--> PowerLoss Boot");
                                }
                                digitalWrite(relays[i], !RelayOn);
                                iot.pub_err("--> Stopping Quick-PowerON");

                        }
                        else{
                                iot.pub_err("--> Continue unfinished processes only");
                        }
                }

                // Erases EEPROM value for HARD_REBOOT
                #if HARD_REBOOT
                EEPROM.write(hReset_eeprom.val_cell, 0);
                EEPROM.commit();
                #endif
                }
}

// ########################### END ADDITIONAL SERVICE ##########################


void setup() {

        #if HARD_REBOOT
        EEPROM.begin(1024);
        check_hardReboot();
        #endif

        startGPIOs();
        quickPwrON();
        startIOTservices();
        startOLED();
        startTO();
}
void loop() {
        iot.looper();

        for (int i = 0; i < NUM_SWITCHES; i++) {
                TO_looper(i);
                checkSwitch_Pressed(i);
        }
        digitalWrite(indic_LEDpin, digitalRead(relays[0]));

        #if USE_RESETKEEPER
        if (checkrebootState == true) {
                recoverReset();
        }
        #endif

        OLEDlooper();


        delay(100);
}
