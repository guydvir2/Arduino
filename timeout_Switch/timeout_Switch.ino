/*
   Upload using Generic ESP8266
   Change default 512 kB ram to
   <<<< 1024 kB of ram !!! MUST!! >>>>
   otherwise OTA will not be loaded next time
 */

#include <myIOT.h>
#include <myJSON.h>
#include <TimeLib.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/nodemcu_test3"

//~~~Services~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_MAN_RESET    false
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false
#define USE_FAT          true // Flash Assist

//~~~Select Board~~~~~~~
#define SONOFF_DUAL      false // <----- Select one
#define SONOFF_BASIC     false // <----- Select one
#define WEMOS            true
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~ define ** onBoot ** timeout in [minutes] ~~~~
/* -1 ==  on
    0 ==  off
    >0 == timeout
 */
int min_timeout[] = {1,2}; //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define VER "SonoffBasic_2.1_alpha"
//####################################################

//~~~~~~~~~~~~~~ myJSON  ~~~~~~~~~~~~~~~~~~~~~
#define jfile "myfile.json"
#define BOOT_CALC_KEY        "BootTime1"
#define BOOT_RESET_KEY       "BootTime2"
#define TIMEOUT0_KEY         "savedTimeOut0"
#define TIMEOUT1_KEY         "savedTimeOut1"
#define SWITCH0_STATE_KEY    "savedSwitch0_State"
#define SWITCH1_STATE_KEY    "savedSwitch1_State"
#define ONTIME0_KEY          "onTime0"
#define ONTIME1_KEY          "onTime1"


// Read from FLASH
long savedBoot_Calc     = 0; // boot time for calc
long savedBoot_reset    = 0; // last actual boot
long savedTimeOut0      = 0; // clock when stopwatch start
long savedTimeOut1      = 0; // clock when stopwatch start
long clockShift         = 0;
long savedOnTime0       = 0;
long savedOnTime1       = 0;


int resetIntervals = 30; //sec time between reboots to be considered as continuation
// int allowedTime_betweenReboots = 99999; // considered as inf time
// int allowedTime_betweenReboots = 0; // considered as every reboot an new operation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
long updated_bootTime = 0;
bool resetBoot_flag   = false;
long end_timeout[]    = {0, 0};
long start_timeout[]  = {0, 0}; // store clock start for TO
int savedTimeOuts []  = {0, 0};

#if (SONOFF_DUAL)

// state definitions
#define RelayOn          HIGH
#define SwitchOn         LOW
#define LedOn            LOW
#define ButtonPressed    LOW

#define RELAY1           5
#define RELAY2           12
#define wifiOn_statusLED 13

#define INPUT1           9
#define INPUT2           0
#define BUTTON           10
#define NUM_SWITCHES     2

#endif


#if (SONOFF_BASIC)
// state definitions
#define RelayOn          HIGH
#define SwitchOn         LOW
#define LedOn            LOW
#define ButtonPressed    LOW

#define RELAY1           12
#define RELAY2           0
#define wifiOn_statusLED 13

#define INPUT1           14
#define INPUT2           0
#define BUTTON           0
#define NUM_SWITCHES     1

#endif


#if (WEMOS)

// state definitions
#define RelayOn       HIGH
#define SwitchOn      LOW
#define LedOn         LOW
#define ButtonPressed LOW

#define RELAY1           14
#define RELAY2           12
#define wifiOn_statusLED 13

#define INPUT1       4
#define INPUT2       5
#define BUTTON       10
#define NUM_SWITCHES 2

#endif


int relays[]         = {RELAY1, RELAY2};
byte inputs[]        = {INPUT1, INPUT2};
char *timeouts[]     = {TIMEOUT0_KEY, TIMEOUT1_KEY}; //  clock to stop device
char *onTime[]       = {ONTIME0_KEY, ONTIME0_KEY}; // clock switched ON
char *switch_state[] = {SWITCH0_STATE_KEY, SWITCH1_STATE_KEY}; // state on or off
int inputs_lastState[NUM_SWITCHES];

char parameters[3][8]; //values from user
char temp_msg[50];

// manual RESET parameters
int manResetCounter               = 0;  // reset press counter
int pressAmount2Reset             = 3; // time to press button to init Reset
long lastResetPress               = 0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
const int deBounceInt             = 50; // mili
// ~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~Start services ~~~~~~~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
myJSON json(jfile, true);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() {

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        if (load_bootTime()) {
                startGPIOs();

                // ~~~~~~~~~~~~~ using switchIt just to notify MQTT  ~~~~~~~
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (min_timeout[i] > 0 ) {
                                switchIt("TimeOut@Boot", i, "on");
                        }
                        else if (min_timeout[i] == -1 ) {
                                switchIt("on@Boot", i, "on");
                        }
                }
                // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        }
        else{
                iot.pub_msg("[Error]: fail to read values from FLASH. RESETING");
                delay(1000);
                iot.sendReset("Flash Error");
        }
}

// ~~~~~~~~~~~~ StartUp ~~~~~~~~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);

                if (min_timeout[i] == 0) {  // OFF STATE
                        digitalWrite(relays[i], !RelayOn);
                        if(USE_FAT) {
                                json.setValue(switch_state[i],0);
                        }
                }
                else if ( min_timeout[i] == -1) { // ON STATE
                        digitalWrite(relays[i], RelayOn);
                        if(USE_FAT) {
                                json.setValue(switch_state[i],1);
                                json.setValue(onTime[i],now());
                        }

                }
                else if (min_timeout[i] > 0) { // TimeOut STATE [minuntes]
                        startBootCounter(i,min_timeout[i]);
                }
                inputs_lastState[i] = digitalRead(inputs[i]);
        }

        if (SONOFF_DUAL) {
                pinMode(BUTTON, INPUT_PULLUP);
                pinMode(wifiOn_statusLED, OUTPUT);
                digitalWrite(wifiOn_statusLED, LedOn);
        }

}
bool startBootCounter(int i, int t){
        if(USE_FAT) {
                if (resetBoot_flag == false) {  // after reset
                        end_timeout[i] = updated_bootTime + t*60;     // seconds
                        json.setValue(switch_state[i],1);
                        json.setValue(timeouts[i],end_timeout[i]);
                        printTime(end_timeout[i],"After BOOT TimeOut : ");
                }
                else if ( json.getValue(timeouts[i], savedTimeOuts[i])) {
                        if (savedTimeOuts[i] == 0 ) {             // No TimeOut Stored
                                end_timeout[i] = updated_bootTime + t*60;           // seconds
                                json.setValue(switch_state[i],1);
                                json.setValue(timeouts[i],end_timeout[i]);
                                json.setValue(onTime[i],now());
                        }
                        else {             // Stored Timeout
                                end_timeout[i] = savedTimeOuts[i];
                        }
                        digitalWrite(relays[i], RelayOn);
                        printTime(end_timeout[i],"TimeOut : ");
                        json.setValue(onTime[i],now());
                        return 1;
                }
                else{ // fail to read from file
                        json.setValue(timeouts[i], 0);
                        printTime(end_timeout[i],"TimeOut : ");
                        return 0;
                        }
        }
        else{
                end_timeout[i] = updated_bootTime + t*60;
                digitalWrite(relays[i], RelayOn);
                return 1;
        }
}
void adhocCounter(int i, int t){
        min_timeout[i]=t;
        if (t>0) {
                end_timeout[i] = now() + t*60;
                switchIt("TimeOut",i,"on");
        }
        else {
                end_timeout[i] = now();
                switchIt("TimeOut",i,"off");
        }

        if(USE_FAT) {
                json.setValue(timeouts[i],end_timeout[i]);
        }
}
void stopCounter(int i){
        digitalWrite(relays[i], !RelayOn);
        end_timeout[i] = now();
        if(USE_FAT) {
                json.setValue(switch_state[i],i);
                json.setValue(timeouts[i],end_timeout[i]);
        }
}
void timeoutLoop() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (end_timeout[i] != 0 ) {
                        if (now() > end_timeout[i]) {
                                switchIt("TimeOut", i, "off");
                                if(USE_FAT) {
                                        json.setValue(timeouts[i],0);
                                }
                                end_timeout[i] = 0;
                        }
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void PBit() {
        int pause = 2 * 5 * deBounceInt;
        allOff();
        delay(pause);

        for (int i = 0; i < NUM_SWITCHES; i++) {
                switchIt("PBit", i, "on");
                delay(pause);
                switchIt("PBit", i, "off");
                delay(pause);
        }

        allOff();
}
void allOff() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                digitalWrite(relays[i], !RelayOn);
                if(USE_FAT) {
                        if(i==0) {
                                json.setValue(SWITCH0_STATE_KEY,0);
                        }
                        else if(i==1) {
                                json.setValue(SWITCH1_STATE_KEY,0);
                        }
                }
                inputs_lastState[i] = digitalRead(inputs[i]);
        }
}
void switchIt(char *type, int sw_num, char *dir) {
        char mqttmsg[50];
        char states[50];
        char tempstr[50];

        if (sw_num < NUM_SWITCHES && sw_num >= 0) {
                if (strcmp(dir, "on") == 0) {
                        digitalWrite(relays[sw_num], RelayOn);
                        if(USE_FAT) {
                                json.setValue(switch_state[sw_num],1);
                        }
                }
                else if (strcmp(dir, "off") == 0) {
                        digitalWrite(relays[sw_num], !RelayOn);
                        if(USE_FAT) {
                                json.setValue(switch_state[sw_num],0);
                        }

                }

                if (iot.mqttConnected) {
                        long timeDiff = now() - end_timeout[sw_num];
                        char time[20];
                        char date[20];
                        convert_epoch2clock(end_timeout[sw_num],now(),time,date);

                        sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num, dir);
                        if (strcmp(type, "TimeOut") == 0) {
                                sprintf(tempstr, " [%d min.]", min_timeout[sw_num]);
                                strcat(mqttmsg, tempstr);
                        }

                        sprintf(states,"");
                        for (int i = 0; i < NUM_SWITCHES; i++) {
                                sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "On" : "Off");
                                strcat(states, tempstr);
                        }
                        iot.pub_state(states);
                        iot.pub_msg(mqttmsg);
                }
        }
}
void detectResetPresses() {
        if (millis() - lastResetPress < timeInterval_resetPress) {
                if (manResetCounter >= pressAmount2Reset) {
                        iot.sendReset("Manual operation");
                        manResetCounter = 0;
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}
void checkSwitch_looper() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (digitalRead(inputs[i]) != inputs_lastState[i]) {
                        delay(deBounceInt);
                        if (digitalRead(inputs[i]) != inputs_lastState[i]) {
                                if (digitalRead(inputs[i]) == SwitchOn) {
                                        switchIt("Button", i, "on");
                                        inputs_lastState[i] = digitalRead(inputs[i]);
                                        if (USE_MAN_RESET && i == 1) {
                                                detectResetPresses();
                                                lastResetPress = millis();
                                        }
                                }
                                else if (digitalRead(inputs[i]) == !SwitchOn) {
                                        switchIt("Button", i, "off");
                                        inputs_lastState[i] = digitalRead(inputs[i]);
                                }
                        }

                }

                else if (USE_BOUNCE_DEBUG) { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "input [%d] Bounce: current state[%d] last state[%d]", i, digitalRead(inputs[i]), inputs_lastState[i]);
                        iot.pub_msg(tMsg);
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~ MQTT  ~~~~~~~~~~~~~~~~~~
void splitter(char *inputstr) {
        char * pch;
        int i = 0;

        pch = strtok (inputstr, " ,.-");
        while (pch != NULL)
        {
                sprintf(parameters[i], "%s", pch);
                pch = strtok (NULL, " ,.-");
                i++;
        }
}
void addiotnalMQTT(char incoming_msg[50]) {
        char state[25];
        char msg[100];
        char tempstr[50];
        char tempstr2[50];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                strcpy(msg, "Status: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "Switch#[%d] [%s] ", i + 1, digitalRead(relays[i]) ? "On" : "Off");
                        strcat(msg, tempstr);
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                strcpy(msg, "Pins: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "Switch#[%d] [%d] ", i + 1, relays[i]);
                        strcat(msg, tempstr);
                }
                if (USE_EXT_BUTTONS) {
                        for (int i = 0; i < NUM_SWITCHES; i++) {
                                sprintf(tempstr2, "Button#[%d] [%d] ", i + 1, inputs[i]);
                                strcat(msg, tempstr2);
                        }
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], MAN_RESET:[%d], EXT_BUTTONS[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_MAN_RESET, USE_EXT_BUTTONS);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, uptime, format, help] , [ver, boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "uptime") == 0 ) {
                long time_left[NUM_SWITCHES];
        //         char aa[10];
        //         char ab[10];
        //         for (int a = 0; a<NUM_SWITCHES; a++) {
        //                 time_left[a] = end_timeout[a]-now();
        //                 if (time_left[a] < 0 ){
        //                   time_left[a] = 0;
        //                 }
        //                 Serial.println(time_left[a]);
        //
        //                 // convert_epoch2clock(time_left[a],0, aa,ab);
        //                 // Serial.println(aa);
        //                 // Serial.println(ab);
        //         }
        //
        //         // sprintf(msg, "UpTime: %02d days  %02d:%02d:%02d", days, hours, minutes, seconds);
        //         iot.pub_msg(msg);
        // }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                sprintf(msg, "Flash Format started");
                json.format();
                iot.pub_msg(msg);
        }
        else { // get user defined definitions
                splitter(incoming_msg);
                if (isDigit(*parameters[1])) { // TimeOut
                        start_timeout[atoi(parameters[0])] = millis();
                        min_timeout[atoi(parameters[0])] = atoi(parameters[1]);
                        switchIt("TimeOut", atoi(parameters[0]), "on");
                }
                else if (strcmp(parameters[1], "on") == 0 || strcmp(parameters[1], "off") == 0) {
                        switchIt("MQTT", atoi(parameters[0]), parameters[1]);
                }
                else if (strcmp(parameters[1], "timeout") == 0) {
                        adhocCounter(atoi(parameters[0]),atoi(parameters[2]));
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~ TIME  txt ~~~~~~~~~~~~~~
void getTime_stamp(time_t t, char *ret_date, char* ret_clock){
        sprintf(ret_date, "%04d-%02d-%02d", year(t), month(t), day(t));
        sprintf(ret_clock, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
void printTime(time_t t, char *text){
        char time1[20];
        char date1[20];
        Serial.print(text);
        getTime_stamp(t,time1, date1);
        Serial.print(time1);
        Serial.print(", ");
        Serial.println(date1);
}
void convert_epoch2clock(long t1, long t2, char* time_str, char* date_str){
        byte days       = 0;
        byte hours      = 0;
        byte minutes    = 0;
        byte seconds    = 0;

        int sec2minutes = 60;
        int sec2hours   = (sec2minutes * 60);
        int sec2days    = (sec2hours * 24);
        int sec2years   = (sec2days * 365);

        long time_delta = t1-t2;

        days    = (int)(time_delta / sec2days);
        hours   = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        sprintf(time_str, "%02d days", days);
        sprintf(date_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~ Load Saved Flash ~~~~~~~~~~~
bool load_bootTime() {
        int suc_counter = 0;

        if (json.getValue(BOOT_CALC_KEY, savedBoot_Calc)) {
                suc_counter+=1;
        }
        else {
                json.setValue(BOOT_CALC_KEY, 0);
        }
        if (json.getValue(BOOT_RESET_KEY, savedBoot_reset)) {
                suc_counter+=1;
        }
        else {
                json.setValue(BOOT_RESET_KEY, 0);
        }

        if (suc_counter == 2) {

                long currentBootTime = now();
                int x =0;
                int maxRetries = 5;


                while (x<maxRetries) { // verify time is updated
                        if (year(currentBootTime) != 1970) { //NTP update succeeded
                                json.setValue(BOOT_RESET_KEY, currentBootTime);
                                int tDelta = currentBootTime - savedBoot_reset;

                                if ( tDelta > resetIntervals ) {
                                        Serial.println("New Boot");
                                        json.setValue(BOOT_CALC_KEY, currentBootTime);
                                        updated_bootTime = currentBootTime;           // take clock of current boot
                                        clockShift = 0;
                                        resetBoot_flag = false;
                                        printTime(updated_bootTime,"BootTime: ");
                                        return 1;
                                }
                                else  {
                                        Serial.println("reset detected");
                                        updated_bootTime = savedBoot_Calc;           // take clock of last boot
                                        clockShift = currentBootTime - updated_bootTime;
                                        resetBoot_flag = true;
                                        printTime(updated_bootTime,"BootTime: ");
                                        return 1;
                                }
                        }
                        else{
                                currentBootTime = now();
                        }
                        x +=1;
                        Serial.println(x);
                        delay(200);
                }
                if (x==maxRetries) { // fail NTP
                        return 0;
                }
        }
        else{
                return 0;
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        timeoutLoop();

        if (iot.mqttConnected) {
                digitalWrite(wifiOn_statusLED, !LedOn);
        }

        if (USE_EXT_BUTTONS) {
                checkSwitch_looper();
        }

        if (SONOFF_DUAL && digitalRead(BUTTON) == ButtonPressed) {
                iot.sendReset("Reset by Button");
        }

        delay(100);
}
