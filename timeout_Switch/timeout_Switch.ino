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

//##############  User Input ##################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/sonoff_test2"
#define VER "SonoffBasic_2.2"

//~~~Services~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false
#define USE_FAT          true // Flash Assist using JSON and FS

//~~~Select Board~~~~~~~
#define SONOFF_DUAL      true // <----- Select one
#define SONOFF_BASIC     false // <----- Select one
#define WEMOS            false
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~ define ** onBoot ** timeout in [minutes] ~~~~
// -1 ==  on, 0 ==  off, >0 == timeout [min]
#define BOOT_TIMEOUT_SW0 0
#define BOOT_TIMEOUT_SW1 2
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//####################################################

//~~~~~~~~~~~~~~ myJSON  ~~~~~~~~~~~~~~~~~~~~~
#define jfile "myfile.json"
// JSON keys for storing values
#define BOOT_CALC_KEY        "BootTime1"
#define BOOT_RESET_KEY       "BootTime2"
#define END_TIMEOUT0_KEY     "savedTimeOut0"
#define END_TIMEOUT1_KEY     "savedTimeOut1"
#define VAL_TIMEOUT0_KEY     "valueTimeOut0"
#define VAL_TIMEOUT1_KEY     "valueTimeOut1"
#define ONTIME0_KEY          "onTime0"
#define ONTIME1_KEY          "onTime1"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~Read from FLASH and store localy ~~~
long savedBoot_Calc   = 0; // clock of boot for calc
long savedBoot_reset  = 0; // clock of last actual boot
long savedTimeOuts [] = {0, 0};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~ Boot and clock adjustments ~~~~~~~
int resetIntervals    = 30; //sec define "new Boot" or "unwanted reset" [0,999999]
long updated_bootTime = 0; // clock of boot ( new boot or recover boot)
bool resetBoot_flag   = false; // flag if newboot, or unwanted reset
long end_timeout[]    = {0, 0}; // clock- when to stop timeout
long on_time[]        = {0, 0}; // clock- when switched on
int min_timeout[]     = {BOOT_TIMEOUT_SW0,BOOT_TIMEOUT_SW1}; //given timeout value [min]
bool inTimeOut []     = {false, false}; // flag when in timeOut mode


// ~~~~ Logic States ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define LedOn            LOW
#define ButtonPressed    LOW

// ~~~~~~ Pins Defs ~~~~~~~~
#define RELAY1           12
#define wifiOn_statusLED 13

// ~~~~ Boards Specific ~~~~
#if (SONOFF_DUAL)
#define RELAY2           5
#define INPUT1           9
#define INPUT2           0
#define BUTTON           10
#define NUM_SWITCHES     2

#endif


#if (SONOFF_BASIC)
// state definitions
#define RELAY2           0
#define INPUT1           14
#define INPUT2           0
#define BUTTON           0
#define NUM_SWITCHES     1
#endif


#if (WEMOS)
#define RELAY2           14
#define wifiOn_statusLED 13

#define INPUT1           4
#define INPUT2           5
#define BUTTON           10
#define NUM_SWITCHES     2
#endif


int relays[]         = {RELAY1, RELAY2};
byte inputs[]        = {INPUT1, INPUT2};
char *timeouts[]     = {END_TIMEOUT0_KEY, END_TIMEOUT1_KEY}; //  clock to stop device
char *val_timeouts[] = {VAL_TIMEOUT0_KEY, VAL_TIMEOUT1_KEY}; //  clock to stop device
char *onTime[]       = {ONTIME0_KEY, ONTIME0_KEY}; // clock switched ON
int inputs_lastState[NUM_SWITCHES];


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

void switchIt(char *type, int sw_num, char *dir, int t = 0) {
        char mqttmsg[50];
        char states[50];
        char tempstr[50];

        if (sw_num < NUM_SWITCHES && sw_num >= 0) {
                if (strcmp(dir, "on") == 0 && digitalRead(relays[sw_num])==!RelayOn) {
                        digitalWrite(relays[sw_num], RelayOn);
                        if(USE_FAT) {
                                json.setValue(onTime[sw_num],now());
                        }
                }
                else if (strcmp(dir, "off") == 0 && digitalRead(relays[sw_num])==RelayOn) {
                        digitalWrite(relays[sw_num], !RelayOn);
                        if (inTimeOut[sw_num]) {
                                inTimeOut[sw_num]=false;
                                end_timeout[sw_num]= now();
                                if(USE_FAT) {
                                        json.setValue(timeouts[sw_num],end_timeout[sw_num]);
                                }
                        }

                }
                else if (strcmp(dir, "timeout") == 0) {
                        digitalWrite(relays[sw_num], RelayOn);
                        if (t>0) {
                                end_timeout[sw_num]= now() +60*t;
                                inTimeOut[sw_num]=true;
                                min_timeout[sw_num]=t;
                                if(USE_FAT) {
                                        json.setValue(timeouts[sw_num],end_timeout[sw_num]);
                                        json.setValue(onTime[sw_num],now());
                                }
                        }
                }

                if (iot.mqttConnected && (strcmp(dir, "timeout") == 0 || strcmp(dir, "on") == 0 || strcmp(dir, "off") == 0)) {
                        char time[20];
                        char date[20];
                        convert_epoch2clock(end_timeout[sw_num],now(),time,date);

                        sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num, dir);
                        if (strcmp(dir, "timeout") == 0) {
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

void setup() {

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
<<<<<<< HEAD
        iot.start_services(ADD_MQTT_FUNC);  // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker

        load_allFLASH_vars();
        startGPIOs();
=======
        iot.start_services(ADD_MQTT_FUNC);

        if (load_bootTime()) {
                startGPIOs();
>>>>>>> updateOIT

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
                delay(100);
                iot.sendReset("Flash Error");
        }
}


// ~~~~~~~~~~~~ StartUp ~~~~~~~~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);

                // ~~~~~ Read flash values for TimeOut ~~~~~~~~~
                int temp_timeout_val[]={0,0};
                if(USE_FAT) {
                        if (json.getValue(val_timeouts[i],temp_timeout_val[i])) {
                                if (temp_timeout_val[i]!=0) {
                                        min_timeout[i]=temp_timeout_val[i];
                                }
                        }
                        else{
                                json.setValue(val_timeouts[i],0);
                        }
                }
                // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                if (min_timeout[i] == 0) {    // OFF STATE
                        digitalWrite(relays[i], !RelayOn);
                }
                else if ( min_timeout[i] == -1) {   // ON STATE
                        digitalWrite(relays[i], RelayOn);
                        if(USE_FAT) {
                                json.setValue(onTime[i],now());
                        }
                }
                else if (min_timeout[i] > 0) {   // TimeOut STATE [minuntes]
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
                        on_time[i]=now();
                        inTimeOut[i] = true;
                        // printTime(end_timeout[i],"After BOOT TimeOut : ");

                        json.setValue(timeouts[i],end_timeout[i]);
                        json.setValue(onTime[i],on_time[i]);
                }

                else if ( json.getValue(timeouts[i], savedTimeOuts[i])) {
                        if (savedTimeOuts[i] == 0 ) {             // No TimeOut Stored
                                end_timeout[i] = updated_bootTime + t*60;           // seconds
                                on_time[i]=now();

                                json.setValue(timeouts[i],end_timeout[i]);
                                json.setValue(onTime[i],on_time[i]);
                        }
                        else {             // Stored Timeout
                                end_timeout[i] = savedTimeOuts[i];
                                json.getValue(onTime[i],on_time[i]);
                                json.getValue(timeouts[i],end_timeout[i]);
                        }

                        inTimeOut[i]=true;
                        digitalWrite(relays[i], RelayOn);
                        // printTime(end_timeout[i],"TimeOut : ");
                        return 1;
                }

                else{ // fail to read from file
                        json.setValue(timeouts[i], 0);
                        // printTime(end_timeout[i],"TimeOut : ");
                        inTimeOut[i]=false;
                        return 0;
                }
        }
        else{
                end_timeout[i] = updated_bootTime + t*60;
                digitalWrite(relays[i], RelayOn);
                inTimeOut[i]=true;
                return 1;
        }
}
void timeoutLoop() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (end_timeout[i] != 0 ) {
<<<<<<< HEAD
                        Serial.println(now());
                        Serial.print(i);
                        Serial.print(" :");
                        Serial.println(end_timeout[i]);
                        if (now() > end_timeout[i] ) {
=======
                        if (now() > end_timeout[i] && digitalRead(relays[i])==RelayOn) {
>>>>>>> updateOIT
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
                inputs_lastState[i] = digitalRead(inputs[i]);
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
void addiotnalMQTT(char *incoming_msg) {
        char state[25];
        char msg[200];
        char tempstr[50];
        char tempstr2[50];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                strcpy(msg, "Status: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "Switch#[%d] [%s], timeOut[%s] ", i, digitalRead(relays[i]) ? "On" : "Off", inTimeOut[i] ? "Yes" : "No");
                        strcat(msg, tempstr);
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                strcpy(msg, "Pins: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "Switch#[%d] pin[%d] ", i, relays[i]);
                        strcat(msg, tempstr);
                }
                if (USE_EXT_BUTTONS) {
                        for (int i = 0; i < NUM_SWITCHES; i++) {
                                sprintf(tempstr2, "Button#[%d] pin[%d] ", i, inputs[i]);
                                strcat(msg, tempstr2);
                        }
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                PBit();
                iot.pub_msg("PowerOnBit");
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], EXT_BUTTONS[%d], FAT[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_EXT_BUTTONS, USE_FAT);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, pins, uptime,remain, timeouts, TOreset, read_all, format, help,[num]-on, [num]-off, [num]-timeout-[min],[num]-TOupdate-[min] ],[ver, boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "uptime") == 0 ) {
                strcpy(msg, "upTime: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (digitalRead(relays[i])==RelayOn) {
                                char time1[20];
                                char date1[20];
                                convert_epoch2clock(now(),on_time[i],time1,date1);
                                sprintf(tempstr, "Switch#[%d] [%s] ", i, time1);
                                strcat(msg, tempstr);
                        }
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "remain") == 0 ) {
                strcpy(msg, "Remain: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (digitalRead(relays[i])==RelayOn && inTimeOut[i]) {
                                char time1[20];
                                char date1[20];
                                convert_epoch2clock(on_time[i]+min_timeout[i]*60,now(), time1,date1);
                                sprintf(tempstr, "Switch#[%d] [%s] ", i, time1);
                                strcat(msg, tempstr);
                        }
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "timeouts") == 0 ) {
                strcpy(msg, "TimeOut: ");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (min_timeout[i]>0) {
                                sprintf(tempstr, "Switch#[%d] [%d min.] ", i, min_timeout[i]);
                                strcat(msg, tempstr);
                        }
                }
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                sprintf(msg, "Flash Format started");
                json.format();
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "TOreset") == 0 ) {
                delete_flash_Timeout();
                sprintf(msg, "TimeOut: Stored TimeOut values deleted. Use defaults");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "read_all") == 0 ) {
                read_all_flash();
                // iot.pub_msg(msg);
        }

        else {
                iot.inline_read (incoming_msg);
                if (atoi(iot.inline_param[0])>=0 && atoi(iot.inline_param[0]) < NUM_SWITCHES) {
                        if (strcmp(iot.inline_param[1], "on") == 0 || strcmp(iot.inline_param[1], "off") == 0) {
                                switchIt("MQTT", atoi(iot.inline_param[0]), iot.inline_param[1]);
                        }
                        else if (strcmp(iot.inline_param[1], "timeout") == 0) { // define counter :1,60
                                switchIt("MQTT",atoi(iot.inline_param[0]),iot.inline_param[1],atoi(iot.inline_param[2]));
                        }
                        else if (strcmp(iot.inline_param[1], "TOupdate") == 0) {
                                update_flash_Timeout(atoi(iot.inline_param[0]),atoi(iot.inline_param[2]));
                        }
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
void convert_epoch2clock(long t1, long t2, char* time_str, char* days_str){
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

        sprintf(days_str, "%02d days", days);
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Load Saved Flash ~~~~~~~~~~~
bool load_bootTime() {
        int suc_counter = 0;
        long clockShift = 0;

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
                int maxRetries = 3;

                while (x<maxRetries) { // verify time is updated
                        if (year(currentBootTime) != 1970) { //NTP update succeeded
                                json.setValue(BOOT_RESET_KEY, currentBootTime);
                                int tDelta = currentBootTime - savedBoot_reset;

                                if ( tDelta > resetIntervals ) {
                                        // Serial.println("New Boot");
                                        json.setValue(BOOT_CALC_KEY, currentBootTime);
                                        updated_bootTime = currentBootTime;           // take clock of current boot
                                        clockShift = 0;
                                        resetBoot_flag = false;
                                        // printTime(updated_bootTime,"BootTime: ");
                                        return 1;
                                }
                                else  {
                                        // Serial.println("reset detected");
                                        updated_bootTime = savedBoot_Calc;           // take clock of last boot
                                        clockShift = currentBootTime - updated_bootTime;
                                        resetBoot_flag = true;
                                        // printTime(updated_bootTime,"BootTime: ");
                                        return 1;
                                }
                        }
                        else{
                                currentBootTime = now();
                        }
                        x +=1;
                        // Serial.println(x);
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
bool update_flash_Timeout(int i, int t){
        char tempMSG[50];
        if( t>0) {
                json.setValue(val_timeouts[i],t);
                sprintf(tempMSG," TimeOut Update: Switch #[%d], [%d min.]",i,t);
                iot.pub_msg(tempMSG);
                return 1;
        }
        else{
                json.setValue(val_timeouts[i],0);
                sprintf(tempMSG," TimeOut Update: Switch #[%d], error upDating timeout",i);
                iot.pub_msg(tempMSG);
                return 0;
        }
}
void delete_flash_Timeout(){
        for (int i = 0; i < NUM_SWITCHES; i++) {
                json.setValue(val_timeouts[i],0);
        }
}
void read_all_flash(){
        json.getValue(BOOT_CALC_KEY, savedBoot_Calc);
        Serial.print("Boot_calc:");
        Serial.println(savedBoot_Calc);

        json.getValue(BOOT_RESET_KEY, savedBoot_reset);
        Serial.print("Boot_reset:");
        Serial.println(savedBoot_reset);

        Serial.print("Boot_updated:");
        Serial.println(updated_bootTime);

        Serial.print("resetIntervals:");
        Serial.println(resetIntervals);

        for(int i=0; i<NUM_SWITCHES; i++) {
                int temp_val;

                Serial.print("hardCoded Timeout #");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(min_timeout[i]);

                json.getValue(val_timeouts[i],temp_val);
                Serial.print("Flash saved Timeout #");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(temp_val);
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
