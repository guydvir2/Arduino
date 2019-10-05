#include <myIOT.h>
#include <myJSON.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "Wemos_4.3"
#define USE_DAILY_TO     true

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      1*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins
int START_dailyTO[3]   ={18,30,0};
int END_dailyTO[3]     ={19,30,0};

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  false
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "WaterBoiler"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~~~~ TimeOuts class ~~~~~~~~~
int TIMEOUTS[2]  = {TIMEOUT_SW0, TIMEOUT_SW1};
timeOUT timeOut_SW0("SW0", TIMEOUTS[0]);
#if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1", TIMEOUTS[1]);
timeOUT *TO[] = {&timeOut_SW0, &timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[] = {&timeOut_SW0};
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~ Use Daily Clock ~~~~
#if USE_DAILY_TO
myJSON dailyTO_inFlash("file0.json", true);
#endif

char *clock_fields[] = {"ontime", "off_time", "flag", "use_inFl_vals"};
int items_each_array[3] = {3, 3, 1};
char *clockAlias = "DailyClock";
struct dTO {
        int on[3];
        int off[3];
        bool flag;
        bool onNow;
        bool useFlash;
};
dTO defaultVals = {{0, 0, 0}, {0, 0, 59}, 0, 0, 0};
dTO dailyTO_0   = {{19, 0, 0}, {20, 0, 0}, 1, 0, 0};
dTO dailyTO_1   = {{20, 0, 0}, {22, 0, 0}, 1, 0, 0};
dTO *dailyTO[]  = {&dailyTO_0, &dailyTO_1};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1          D5
#define RELAY2          D2
#define INPUT1          D6
#define INPUT2          D5
#define buttonLED_Pin   D7

byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define ledON            HIGH

bool relState[NUM_SWITCHES];
bool last_relState[NUM_SWITCHES];
//##########################

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
int timeIncrements           = 1; //minutes each button press
int timeInc_counter          = 0; // counts number of presses to TO increments
int delayBetweenPress        = 500; // consequtive presses to reset
int deBounceInt              = 50;
int time_NOref_OLED          = 10; // seconds to stop refresh OLED
unsigned long clock_noref    = 0;
unsigned long pressTO_input1 = 0; // TimeOUT for next press
unsigned long startTime      = 0;
unsigned long endTime        = 0;
// ##########################

char msg[150];


void switchIt (char *txt1, int sw_num, bool state, char *txt2 = "", bool show_timeout = true) {
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word = {"Turned"};

        if (digitalRead(relays[sw_num]) != state ) {
                digitalWrite(relays[sw_num], state);
                TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(), now(), msg1, msg2);
                sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
                if (state == 1 && show_timeout) {
                        sprintf(msg2, "timeLeft[%s]", msg1);
                        strcat(msg, msg2);
                }
                iot.pub_msg(msg);

                sprintf(states, "");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "ON" : "OFF");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
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
        pinMode(buttonLED_Pin, OUTPUT);
}

// ~~~~~~ DailyTimeOuts ~~~~~~~
void timeOutLoop(byte i) {
        char msg_t[50], msg[50];

        relState[i] = TO[i]->looper();
        if (relState[i] != last_relState[i]) {         // change state (ON <-->OFF)
                if (relState[i]==0) { // when TO ends
                        display_totalOnTime();
                        TO[i]->updateStart(0);
                }
                else{ // when TO starts
                        TO[i]->updateStart(now());
                }
                switchIt("TimeOut", i, relState[i]);
        }
        last_relState[i] = relState[i];
}
#if USE_DAILY_TO
void daily_timeouts_looper(dTO &dailyTO, byte i = 0) {
        char msg [50], msg2[50];
        time_t t = now();

        if (dailyTO.onNow == false && dailyTO.flag == true) { // start
                if (hour(t) == dailyTO.on[0] && minute(t) == dailyTO.on[1] && second(t) == dailyTO.on[2]) {
                        int secs   = dailyTO.off[2] - dailyTO.on[2];
                        int mins   = dailyTO.off[1] - dailyTO.on[1];
                        int delt_h = dailyTO.off[0] - dailyTO.on[0];

                        int total_time = secs + mins * 60 + delt_h * 60 * 60;
                        if (total_time < 0) {
                                total_time += 24 * 60 * 60;
                        }

                        TO[i]->setNewTimeout(total_time, false);
                        dailyTO.onNow = true;
                }
        }
}
void check_dailyTO_inFlash(dTO &dailyTO, int x) {
        char temp[10];
        int retVal;

        if (dailyTO_inFlash.file_exists()) {
                sprintf(temp, "%s_%d", clock_fields[3], x);
                dailyTO_inFlash.getValue(temp, retVal);
                if (retVal) { //only if flag is to read values from flash
                        for (int m = 0; m < sizeof(clock_fields) / sizeof(clock_fields[0]); m++) {
                                sprintf(temp, "%s_%d", clock_fields[m], x);
                                if (m == 0 || m == 1) { // clock fileds only -- on or off
                                        for (int i = 0; i < items_each_array[m]; i++) {
                                                dailyTO_inFlash.getArrayVal(temp, i, retVal);
                                                if (retVal >= 0) {
                                                        if ((i == 0 && retVal <= 23) || (i > 0 && retVal <= 59)) { //valid time
                                                                if ( m == 0) {
                                                                        dailyTO.on[i] = retVal;
                                                                }
                                                                else {
                                                                        dailyTO.off[i] = retVal;
                                                                }
                                                        }
                                                }

                                        }
                                }
                                else { // for flag value
                                        dailyTO_inFlash.getValue(temp, retVal);
                                        if (retVal == 0 || retVal == 1) { //flag on or off
                                                if (m == 2) {
                                                        dailyTO.flag = retVal;
                                                }
                                                else if (m == 3) {
                                                        dailyTO.useFlash = retVal;
                                                }
                                        }
                                        else {
                                                dailyTO_inFlash.setValue(temp, 0);
                                        }
                                }
                        }

                }
        }
        else {         // create NULL values
                store_dailyTO_inFlash(defaultVals, x);
        }

}
void store_dailyTO_inFlash(dTO &dailyTO, int x) {
        char temp[10];

        for (int m = 0; m < sizeof(clock_fields) / sizeof(clock_fields[0]); m++) {
                sprintf(temp, "%s_%d", clock_fields[m], x);
                if (m == 0) {
                        for (int i = 0; i < items_each_array[m]; i++) {
                                dailyTO_inFlash.setArrayVal(temp, i, dailyTO.on[i]);
                        }
                }
                else if (m == 1) {
                        for (int i = 0; i < items_each_array[m]; i++) {
                                dailyTO_inFlash.setArrayVal(temp, i, dailyTO.off[i]);
                        }
                }
                else if (m == 2) {
                        dailyTO_inFlash.setValue(temp, dailyTO.flag);
                }
                else if (m == 3) {
                        dailyTO_inFlash.setValue(temp, dailyTO.useFlash);

                }
        }
}
#endif


void setup() {
        startGPIOs();
        startIOTservices();
        startOLED();
        TO[0]->begin(false);

#if USE_DAILY_TO
        for (int i = 0; i < NUM_SWITCHES; i++) {
                check_dailyTO_inFlash(*dailyTO[i], i);
        }
#endif
}

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

        if( clock_noref == 0) {

                if (digitalRead(relays[0]) == RelayOn ) {
                        int timeON = now() - TO[0]->getStart_to();
                        sec2clock(timeON, "", time_on_char);
                        if ( timeInc_counter == 1 ) { // ~~~~ON, no timer ~~~~~~~
                                OLED_SideTXT(2, "On:", time_on_char);
                        }
                        else if ( timeInc_counter > 1 || relState[0] == 1 ) { /// ON + Timer or DailyTO
                                int timeLeft = TO[0]->remain();
                                sec2clock(timeLeft, "", time2Off_char);
                                OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
                        }
                }
                else { // OFF state - clock only
                        iot.return_clock(timeStamp);
                        iot.return_date(dateStamp);

                        int timeQoute = 5000;

                        if (swapLines_counter == 0) {
                                swapLines_counter = millis();
                        }
                        if (millis() - swapLines_counter < timeQoute) {
                                OLED_CenterTXT(2, "", timeStamp, dateStamp);
                        }
                        else if (millis() - swapLines_counter >= timeQoute && millis() - swapLines_counter < 2 * timeQoute)
                        {
                                OLED_CenterTXT(2, timeStamp, "", "", dateStamp);
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
        int totalONtime = now() - TO[0]->getStart_to();
        sec2clock(totalONtime, "", msg);
        clock_noref = millis();
        OLED_CenterTXT(2,"ON time:","",msg);
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void Switch_1_looper(byte i) {
        char tempstr[20];
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
void addiotnalMQTT(char *incoming_msg) {
        char msg[150];
        char msg2[20];
        if      (strcmp(incoming_msg, "status") == 0) {
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (TO[i]->remain() > 0) {
                                TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), msg, msg2);
                                sprintf(msg2, "timeLeft[%s]", msg);
                        }
                        else {
                                sprintf(msg2, "");
                        }
                        sprintf(msg, "Status: Switch[#%d] [%s] %s", i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
                sprintf(msg, "ver #2: DailyTO[%d]",USE_DAILY_TO);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to, status_TO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #4 - [off_daily_to, on_daily_to, flag_daily_to, useflash_daily_to, status_daily_to]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "flash") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.format();
        }
        else {
                iot.inline_read(incoming_msg);

                if (strcmp(iot.inline_param[1], "on") == 0 ) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 1);
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 0);
                }
                else if (strcmp(iot.inline_param[1], "timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg);
                        delay(1000);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut update");
                }
                else if (strcmp(iot.inline_param[1], "remain") == 0) {
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restart_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch[#%d] [Restart]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                        // iot.notifyOffline();
                        // iot.sendReset("TimeOut restart");
                }
                else if (strcmp(iot.inline_param[1], "status_TO") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
                                "TimeOut", atoi(iot.inline_param[0]),
                                TIMEOUTS[atoi(iot.inline_param[0])],
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "end_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->endNow();
                        sprintf(msg, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restore_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->restore_to();
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch[#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
                        iot.pub_msg(msg);
                        iot.notifyOffline();
                        iot.sendReset("Restore");
                }
# if USE_DAILY_TO
                else if (strcmp(iot.inline_param[1], "on_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->on[0] = atoi(iot.inline_param[2]); // hour
                        dailyTO[atoi(iot.inline_param[0])]->on[1] = atoi(iot.inline_param[3]); // minute
                        dailyTO[atoi(iot.inline_param[0])]->on[2] = atoi(iot.inline_param[4]); // seconds

                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])], atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch[#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->on[0],
                                dailyTO[atoi(iot.inline_param[0])]->on[1],
                                dailyTO[atoi(iot.inline_param[0])]->on[2]);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "off_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->off[0] = atoi(iot.inline_param[2]); // hour
                        dailyTO[atoi(iot.inline_param[0])]->off[1] = atoi(iot.inline_param[3]); // minute
                        dailyTO[atoi(iot.inline_param[0])]->off[2] = atoi(iot.inline_param[4]); // seconds

                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])], atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->off[0],
                                dailyTO[atoi(iot.inline_param[0])]->off[1],
                                dailyTO[atoi(iot.inline_param[0])]->off[2]);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "flag_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->flag = atoi(iot.inline_param[2]);
                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])], atoi(iot.inline_param[0]));
                        sprintf(msg, "%s: Switch[#%d] set to [%s]", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "useflash_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->useFlash = atoi(iot.inline_param[2]);
                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])], atoi(iot.inline_param[0]));
                        sprintf(msg, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "status_daily_to") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {ON, %02d:%02d:%02d} {OFF, %02d:%02d:%02d} {Flag: %s}",
                                clockAlias, atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->on[0],
                                dailyTO[atoi(iot.inline_param[0])]->on[1],
                                dailyTO[atoi(iot.inline_param[0])]->on[2],
                                dailyTO[atoi(iot.inline_param[0])]->off[0],
                                dailyTO[atoi(iot.inline_param[0])]->off[1],
                                dailyTO[atoi(iot.inline_param[0])]->off[2],
                                dailyTO[atoi(iot.inline_param[0])]->flag ? "ON" : "OFF" );
                        iot.pub_msg(msg);
                }
# endif
        }
}

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        iot.looper();
        OLEDlooper();
        digitalWrite(buttonLED_Pin, digitalRead(relays[0]));

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (USE_DAILY_TO == true) {
                    #if USE_DAILY_TO
                        daily_timeouts_looper(*dailyTO[i], i);
                    #endif
                }
                timeOutLoop(i);
                Switch_1_looper(i);
        }
        delay(100);
}
