#include <myIOT.h>
#include <myJSON.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "Wemos_4.0"
#define STATE_AT_BOOT    false // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO     false
#define USE_OLED         true

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      1*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
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
dTO dailyTO_0   = {{18, 0, 0}, {19, 0, 0}, 1, 0, 0};
dTO dailyTO_1   = {{20, 00, 0}, {22, 0, 0}, 1, 0, 0};
dTO *dailyTO[]  = {&dailyTO_0, &dailyTO_1};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1          D6
#define RELAY2          D2
#define INPUT1          D7
#define INPUT2          D5

#define buttonLED_Pin   D8

byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ ResetKeeper Vars ~~~~~~~
int rebootState         = 0;
bool checkrebootState   = true;
bool boot_overide       = true;

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define ledON            HIGH

bool swState[NUM_SWITCHES];
bool last_swState[NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];
bool relayState;
//##########################

// ~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~~~~~~~
#if USE_OLED

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET    LED_BUILTIN
//  int SCLpin       = D1;
//  int SDApin       = D2;
long swapLines_counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// TimeOut Constants
int maxTO                    = 150; //minutes to timeout even in ON state
int timeIncrements           = 15; //minutes each button press
int timeInc_counter          = 0; // counts number of presses to TO increments
unsigned long startTime      = 0;
unsigned long endTime        = 0;
int delayBetweenPress        = 500; // consequtive presses to reset
unsigned long pressTO_input1 = 0; // TimeOUT for next press
const int deBounceInt        = 50;
// ##########################

char timeStamp [50];
char dateStamp [50];
char msg[150];
char parameters [2][4];



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
// void checkSwitch_Pressed (byte sw, bool momentary = true) {
//         if (momentary) {
//                 if (digitalRead(inputs[sw]) == LOW) {
//                         delay(50);
//                         if (digitalRead(inputs[sw]) == LOW) {
//                                 if (digitalRead(relays[sw]) == RelayOn) {
//                                         TO[sw]->endNow();
//                                 }
//                                 else {
//                                         TO[sw]->restart_to();
//                                 }
//                                 delay(500);
//                         }
//                 }
//         }
//         else {
//                 if (digitalRead(inputs[sw]) != inputs_lastState[sw]) {
//                         delay(50);
//                         if (digitalRead(inputs[sw]) != inputs_lastState[sw]) {
//                                 inputs_lastState[sw] = digitalRead(inputs[sw]);
//                                 if (digitalRead(inputs[sw]) == SwitchOn) {
//                                         TO[sw]->restart_to();
//                                 }
//                                 else {
//                                         TO[sw]->endNow();
//                                 }
//                         }
//                 }
//         }
// }



void startIOTservices() {
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC,"Xiaomi_ADA6","guyd5161");
}
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);
                inputs_lastState[i] = digitalRead(inputs[i]);

                swState [i] = 0;
                last_swState [i] = 0;
        }
        pinMode(buttonLED_Pin, OUTPUT);
}

// ~~~~~~ DailyTimeOuts ~~~~~~~
void timeOutLoop(byte i) {
        char msg_t[50], msg[50];

        if (iot.mqtt_detect_reset != 2) {
                swState[i] = TO[i]->looper();
                if (swState[i] != last_swState[i]) { // change state (ON <-->OFF)
                        switchIt("TimeOut", i, swState[i]);
                }
                last_swState[i] = swState[i];
        }
}
#if USE_DAILY_TO
void daily_timeouts_looper(dTO &dailyTO, byte i = 0) {
        char msg [50], msg2[50];
        time_t t = now();

        if (dailyTO.onNow == false && dailyTO.flag == true) {
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
        else if (hour(t) == dailyTO.off[0] && minute(t) == dailyTO.off[1] && second(t) == dailyTO.off[2] && digitalRead(relays[i]) == RelayOn) {
                TO[i]->endNow();

                dailyTO.onNow = false;
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
                else { // create NULL values
                        store_dailyTO_inFlash(defaultVals, x);
                }
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
        // quickPwrON();
        startIOTservices();
        // TO[0]->begin(false);

        startOLED();

#if USE_DAILY_TO
        for (int i = 0; i < NUM_SWITCHES; i++) {
                check_dailyTO_inFlash(*dailyTO[i], i);
        }
#endif
}


// ~~~~ OLED ~~~~~~~
void startOLED() {
#if (USE_OLED)
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
#endif
}
void OLED_CenterTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0,  byte y_shift = 0) {
#if (USE_OLED)
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
#endif
}
void OLED_SideTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "") {
#if (USE_OLED)
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
#endif
}
void OLEDlooper() {
#if (USE_OLED)
        char time_on_char[20];
        char time2Off_char[20];

        if (last_swState[0] == RelayOn ) {
                long startT;
                TO[0]->getStart_to(startT);
                int timeON = now() - startT;
                sec2clock(timeON, "", time_on_char);
                if ( timeInc_counter == 1 ) { // ~~~~ON, no timer ~~~~~~~
                        OLED_SideTXT(2, "On:", time_on_char);
                }
                else if ( timeInc_counter > 1 ) { /// ON + Timer
                        int timeLeft = TO[0]->remain() - now();
                        sec2clock(timeLeft, "", time2Off_char);
                        OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
                }
        }
        else {   // OFF state - clock only
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

#endif
}
// ~~~~~~~~~~~~~~~

// ~~~~ string creation ~~~~~~
void sec2clock(int sec, char* text, char* output_text) {
        int h = ((int)(sec) / (1000 * 60 * 60));
        int m = ((int)(sec) - h * 1000 * 60 * 60) / (1000 * 60);
        int s = ((int)(sec) - h * 1000 * 60 * 60 - m * 1000 * 60) / 1000;
        sprintf(output_text, "%s %01d:%02d:%02d", text, h, m, s);
}
int splitter(char *inputstr) {
        char * pch;
        int i = 0;

        pch = strtok (inputstr, " ,.-");
        while (pch != NULL)
        {
                sprintf(parameters[i], "%s", pch);
                pch = strtok (NULL, " ,.-");
                i++;
        }
        return i;
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void allOff() {
        digitalWrite(RELAY1, !RelayOn);
        digitalWrite(buttonLED_Pin, !ledON);
}
void switchIt(char *type, char *dir) {
        bool states[2];
        bool suc_flag = false;
        char mqttmsg[50];

        if (strcmp(dir, "on") == 0) {
                if (digitalRead(RELAY1) != RelayOn) { // was not ON
                        digitalWrite(RELAY1, RelayOn);
                        digitalWrite(buttonLED_Pin, ledON);
                        if (startTime == 0) {
                                startTime = millis();
                        }
                }
                else { // case of switching from TO mode to normal ON
                        endTime = 0;
                }
                suc_flag = true;
        }
        else if (strcmp(dir, "off") == 0 && digitalRead(RELAY1) == RelayOn) {
                digitalWrite(RELAY1, !RelayOn);
                digitalWrite(buttonLED_Pin, !ledON);
                startTime = 0;
                timeInc_counter = 0;
                endTime = 0;

                suc_flag = true;
        }

        if (suc_flag) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
        }
}

void Switch_1_looper() {
        if (digitalRead(INPUT1) == SwitchOn) {
                delay(deBounceInt);
                // CASE #1 : Button is pressed. Delay creates a delay when buttons is pressed constantly
                if (digitalRead(INPUT1) == SwitchOn && millis() - pressTO_input1 > delayBetweenPress) {
                        // CASE of it is first press and Relay was off - switch it ON, no timer.
                        if ( timeInc_counter == 0 && last_swState[0] == !RelayOn ) { // first press turns on
                                switchIt("Button", 0, 1, 0);
                                timeInc_counter += 1;
                        }
                        // CASE of already on, and insde interval of time - to add timer Qouta
                        else if (timeInc_counter < (maxTO / timeIncrements) && (millis() - pressTO_input1) < 2500 ) { // additional presses update timer countdown
                                endTime = timeInc_counter * timeIncrements * 1000 * 60 + startTime;
                                int newTO = TO[0]->remain() + timeInc_counter * timeIncrements * 60;
                                TO[0]->setNewTimeout(newTO);

                                sec2clock((timeInc_counter) * timeIncrements * 1000 * 60, "Added Timeout: +", msg);
                                timeInc_counter += 1; // Adding time Qouta
                                iot.pub_msg(msg);
                        }
                        // CASE of time is begger that time out-  sets it OFF
                        else if (timeInc_counter >= (maxTO / timeIncrements) || (millis() - pressTO_input1) > 2500) { // Turn OFF
                                TO[0]->endNow();
                                startTime = 0;
                                timeInc_counter = 0;
                                endTime = 0;
                                // switchIt("Button", "off");
                        }
                        pressTO_input1 = millis();
                }
        }
}
void addiotnalMQTT(char *incoming_msg) {
        char msg[100];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: Relay:[%s], Sw:[%s]", digitalRead(RELAY1) ? "Off" : "On", digitalRead(INPUT1) ? "Off" : "On");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: [%d], Relay: [%d], Led: [%d]", INPUT1, RELAY1, buttonLED_Pin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], OLED[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_OLED);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0 ) {
                switchIt("MQTT", incoming_msg);
                timeInc_counter = 1; // ON only
        }
        else if (strcmp(incoming_msg, "off") == 0 ) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "remain") == 0 ) {
                char remTime[40];
                if (endTime == 0) {
                        sec2clock(0, "Remain Time: ", remTime);
                }
                else if (endTime > 0) {
                        sec2clock(endTime - millis(), "Remain Time: ", remTime);
                }
                iot.pub_msg(remTime);
        }
        else {
                int len = splitter (incoming_msg);
                if (len == 2 && strcmp(parameters[0], "on") == 0 && atoi(parameters[1])) {
                        timeInc_counter = atoi(parameters[1]); /// timeIncrements; // ON + Timer
                        switchIt("MQTT", "on");
                        sec2clock(timeInc_counter * 1000 * 60, "Added Timeout: +", msg);
                        iot.pub_msg(msg);
                        endTime = timeInc_counter * 1000 * 60 + startTime;
                }
        }
}

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        iot.looper();
        OLEDlooper();
        Switch_1_looper();

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (USE_DAILY_TO == true) {
                    #if USE_DAILY_TO
                        daily_timeouts_looper(*dailyTO[i], i);
                    #endif
                }
                timeOutLoop(i);
        }

        digitalWrite(buttonLED_Pin, digitalRead(relays[0]));
        delay(100);
}
