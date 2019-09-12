/* For SonOff Devices:
   1) Upload using Generic ESP8266
   2) Change default 1024kB (64kB SPIFFS ) ram
   <<<<  !!! MUST!! >>>>
   otherwise OTA will not be loaded next time
 */

#include <myIOT.h>
#include <myJSON.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <EEPROM.h>

// ********** Sketch Services  ***********
#define VER              "Sonoff_3.0"
#define USE_INPUTS       false
#define STATE_AT_BOOT    false // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO     true
#define IS_SONOFF        true
#define HARD_REBOOT      true
// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      3*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins
// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "test2"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "OutdoorLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ~~~~~~~ TimeOuts class ~~~~~~~~~
int TIMEOUTS[2]  = {TIMEOUT_SW0,TIMEOUT_SW1};
timeOUT timeOut_SW0("SW0",TIMEOUTS[0]);
#if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1",TIMEOUTS[1]);
timeOUT *TO[]={&timeOut_SW0,&timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[]={&timeOut_SW0};
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~ Use Daily Clock ~~~~
#if USE_DAILY_TO
myJSON dailyTO_inFlash("file0.json", true);
#endif

char *clock_fields[] = {"ontime", "off_time", "flag"};
int items_each_array[3] = {3,3,1};
char *clockAlias = "DailyClock";
struct dTO {
        int on[3];
        int off[3];
        bool flag;
        bool onNow;
};
dTO defaultVals = {{0,0,0},{0,0,0},0,0};
dTO dailyTO_0   = {{19,0,0},{6,30,0},1,0};
dTO dailyTO_1   = {{20,30,0},{23,0,0},1,0};
dTO *dailyTO[]  = {&dailyTO_0,&dailyTO_1};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
// ~~~~ HW Pins and Statdes ~~~~
#if IS_SONOFF
#define RELAY1          12
#define RELAY2          5
#define INPUT1          0
#define INPUT2          14
#endif

#if !IS_SONOFF
#define RELAY1          D2
#define RELAY2          5
#define INPUT1          9
#define INPUT2          3
#endif

#define LEDpin          13
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

bool swState[NUM_SWITCHES];
bool last_swState[NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];
//####################################################

void switchIt (char *txt1, int sw_num, bool state, char *txt2=""){
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word={"Turned"};

        if(digitalRead(relays[sw_num])!= state || boot_overide == true) {
                digitalWrite(relays[sw_num], state);
                TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(),now(), msg1, msg2);
                if (boot_overide == true && iot.mqtt_detect_reset == 1) { //BOOT TIME ONLY for after quick boot
                        word = {"Resume"};
                        boot_overide = false;
                }
                sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
                if (state==1) {
                        sprintf(msg2,"timeLeft[%s]", msg1);
                        strcat(msg, msg2);
                }

                iot.pub_msg(msg);

                sprintf(states,"");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "ON" : "OFF");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
        }
}
void checkSwitch_Pressed (byte sw, bool momentary=true){
        if (momentary) {
                if (digitalRead(inputs[sw])==LOW) {
                        delay(50);
                        if (digitalRead(inputs[sw]) == LOW) {
                                if (digitalRead(relays[sw]) == RelayOn) {
                                        TO[sw]->endNow();
                                }
                                else {
                                        TO[sw]->restart_to();
                                }
                                delay(500);
                        }
                }
        }
        else{
                if (digitalRead(inputs[sw]) !=inputs_lastState[sw]) {
                        delay(50);
                        if (digitalRead(inputs[sw]) !=inputs_lastState[sw]) {
                                inputs_lastState[sw] = digitalRead(inputs[sw]);
                                if (digitalRead(inputs[sw]) == SwitchOn) {
                                        TO[sw]->restart_to();
                                }
                                else{
                                        TO[sw]->endNow();
                                }
                        }
                }
        }
}

void startIOTservices(){
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);//,"GuyDvir","guyd5161");
}
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);

                if (USE_INPUTS) {
                        inputs_lastState[i] = digitalRead(inputs[i]);
                }

                swState [i] = 0;
                last_swState [i] = 0;
        }
        if (IS_SONOFF) {
                pinMode(LEDpin, OUTPUT);
        }
}
void check_hardReboot(byte i=1, byte threshold = 2){
        hReset_eeprom.jump = EEPROM.read(0);

        hReset_eeprom.val_cell    = hReset_eeprom.jump + i;
        hReset_eeprom.wcount_cell = hReset_eeprom.val_cell + 1;

        hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);
        hReset_eeprom.wcount = EEPROM.read(hReset_eeprom.wcount_cell);

        // Serial.print("jump_value: ");
        // Serial.println(hReset_eeprom.jump);

        // if (hReset_eeprom.wcount > 15) {
        //         EEPROM.write(0, hReset_eeprom.jump + 2);
        //         EEPROM.commit();
        // }

        if (hReset_eeprom.val < threshold) {
                EEPROM.write(hReset_eeprom.val_cell,hReset_eeprom.val+1);
                EEPROM.commit();
                // EEPROM.write(hReset_eeprom.wcount_cell,hReset_eeprom.wcount+1);
                // EEPROM.commit();
                hReset_eeprom.hBoot = false;
        }
        else {
                // EEPROM.write(hReset_eeprom.val_cell,0);
                // EEPROM.commit();
                // EEPROM.write(hReset_eeprom.wcount_cell,hReset_eeprom.wcount+1);
                // EEPROM.commit();

                // Serial.println("RESET");
                hReset_eeprom.hBoot = true;
        }
}
void quickPwrON(){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */

        // on_using_hBoot = check_hardReboot(0);
        hReset_eeprom.hBoot = false;
        if (HARD_REBOOT) {
                check_hardReboot();
        }

        for(int i=0; i<NUM_SWITCHES; i++) {
                if (TO[i]->endTO_inFlash || STATE_AT_BOOT || hReset_eeprom.hBoot) {
                        digitalWrite(relays[i], HIGH);
                }
                else{
                        digitalWrite(relays[i], LOW);
                }
        }
}
void recoverReset(){
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

        if(rebootState != 2) { // before getting online/offline MQTT state
                checkrebootState = false;
                for (int i=0; i<NUM_SWITCHES; i++) {
                        if (rebootState == 0 || hReset_eeprom.hBoot ) { //}|| ) {  // PowerOn - not a quickReboot
                                TO[i]->restart_to();
                        }
                        else { // prevent quick boot to restart after succsefull end
                                if (TO[i]->begin(false) == 0) { // if STATE_AT_BOOT == true turn off if not needed
                                        digitalWrite(relays[i],LOW);
                                }
                                iot.pub_err("--> Quick-Reset");
                        }
                }
        }
}

void timeOutLoop(byte i){
        char msg_t[50], msg[50];

        if(iot.mqtt_detect_reset != 2) {
                swState[i] = TO[i]->looper();
                if (swState[i]!=last_swState[i]) {         // change state (ON <-->OFF)
                        switchIt("TimeOut",i,swState[i]);
                        if (IS_SONOFF) {
                                digitalWrite(LEDpin,!swState[i]);
                        }
                }
                last_swState[i] = swState[i];
        }
}
void daily_timeouts_looper(dTO &dailyTO, byte i=0){
        char msg [50], msg2[50];
        time_t t=now();

        if (dailyTO.onNow == false && dailyTO.flag == true) {
                if (hour(t) == dailyTO.on[0] && minute(t) == dailyTO.on[1] && second(t) == dailyTO.on[2]) {
                        int secs   = dailyTO.off[2] - dailyTO.on[2];
                        int mins   = dailyTO.off[1] - dailyTO.on[1];
                        int delt_h = dailyTO.off[0] - dailyTO.on[0];

                        int total_time = secs + mins*60 + delt_h*60*60;
                        if (total_time < 0) {
                                total_time +=24*60*60;
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
void check_dailyTO_inFlash(dTO &dailyTO, int x){
        char temp[10];
        int retVal;

        if (dailyTO_inFlash.file_exists()) {
                for(int m=0; m<sizeof(clock_fields)/sizeof(clock_fields[0]); m++) {
                        sprintf(temp,"%s_%d",clock_fields[m], x);
                        Serial.println(temp);
                        dailyTO_inFlash.printFile();

                        if (m == 0 || m == 1) { // clock fileds only
                          // Serial.print("in flash:");

                                for(int i=0; i<items_each_array[m]; i++) {
                                        dailyTO_inFlash.getArrayVal(temp,i,retVal);

                                        // Serial.println(retVal);
                                        if (retVal !=-1 && retVal >=0 && retVal <=59) { //valid time
                                                if ( m == 0) {
                                                        if (retVal !=dailyTO.on[i]) {
                                                                dailyTO.on[i] = retVal;
                                                        }
                                                }
                                                else {
                                                        if (retVal !=dailyTO.off[i]) {
                                                                dailyTO.off[i] = retVal;
                                                        }
                                                }
                                        }
                                        else {
                                                dailyTO_inFlash.setArrayVal(temp,i,0);
                                                Serial.println("A");
                                        }
                                }
                        }
                        else {               // for flag value
                                dailyTO_inFlash.getValue(temp, retVal);
                                if (retVal == 0 || retVal == 1) { //flag on or off
                                        if (retVal !=dailyTO.flag) {
                                                dailyTO.flag = retVal;
                                        }
                                }
                                else {
                                        dailyTO_inFlash.setValue(temp,0);
                                }
                        }
                }
        }
        else{ // create NULL values
                store_dailyTO_inFlash(defaultVals,0);
        }
}
void store_dailyTO_inFlash(dTO &dailyTO, int x){
        char temp[10];

        for(int m=0; m<sizeof(clock_fields)/sizeof(clock_fields[0]); m++) {
                sprintf(temp,"%s_%d",clock_fields[m], x);
                if (m==0) {
                        for(int i=0; i<items_each_array[m]; i++) {
                                dailyTO_inFlash.setArrayVal(temp,i,dailyTO.on[i]);
                        }
                }
                else if (m==1) {
                        for(int i=0; i<items_each_array[m]; i++) {
                                dailyTO_inFlash.setArrayVal(temp,i,dailyTO.off[i]);
                        }
                }
                else if (m==2) {
                        dailyTO_inFlash.setValue(temp,dailyTO.flag);
                }
        }
}

void addiotnalMQTT(char incoming_msg[50]) {
        char msg[150];
        char msg2[20];
        if      (strcmp(incoming_msg, "status") == 0) {
                for(int i=0; i<NUM_SWITCHES; i++) {
                        if(TO[i]->remain()>0) {
                                TO[i]->convert_epoch2clock(now()+TO[i]->remain(),now(), msg, msg2);
                                sprintf(msg2,"timeLeft[%s]", msg);
                        }
                        else{
                                sprintf(msg2,"");
                        }
                        sprintf(msg, "Status: Switch[#%d] [%s] %s",i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
                sprintf(msg, "ver #2: DailyTO[%d], UseInputs[%d], STATE_AT_BOOT[%d], IS_SONOFF[%d]",USE_DAILY_TO, USE_INPUTS, STATE_AT_BOOT, IS_SONOFF);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to, status_TO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #4 - [off_daily_to, on_daily_to, flag_daily_to]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "flash") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.format();
        }
        else{
                iot.inline_read(incoming_msg);

                if (strcmp(iot.inline_param[1],"on") == 0 ) {
                        switchIt("MQTT",atoi(iot.inline_param[0]),1);
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT",atoi(iot.inline_param[0]),0);
                }
                else if(strcmp(iot.inline_param[1],"timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+atoi(iot.inline_param[2])*60,now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]),msg2);
                        iot.pub_msg(msg);
                }
                else if(strcmp(iot.inline_param[1],"updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg, "TimeOut: Switch[%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg);
                        delay(1000);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut update");
                }
                else if (strcmp(iot.inline_param[1], "remain") == 0) {
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+ TO[atoi(iot.inline_param[0])]->remain(),now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] Remain [%s]",atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restart_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch[#%d] [Restart]",atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut restart");
                }
                else if (strcmp(iot.inline_param[1], "status_TO") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
                                "TimeOut",atoi(iot.inline_param[0]),
                                TIMEOUTS[atoi(iot.inline_param[0])],
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "end_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->endNow();
                        sprintf(msg, "TimeOut: Switch[#%d] [Abort]",atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restore_to") == 0) {
                        TO[atoi(iot.inline_param[0])]->restore_to();
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch[#%d], Restore hardCoded Value [%d mins.]",atoi(iot.inline_param[0]), TIMEOUT_SW0);
                        iot.pub_msg(msg);
                        iot.notifyOffline();
                        iot.sendReset("Restore");
                }
                else if (strcmp(iot.inline_param[1], "on_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->on[0]=atoi(iot.inline_param[2]); // hour
                        dailyTO[atoi(iot.inline_param[0])]->on[1]=atoi(iot.inline_param[3]); // minute
                        dailyTO[atoi(iot.inline_param[0])]->on[2]=atoi(iot.inline_param[4]); // seconds

                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])],atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Clock[#%d] [ON] updated [%02d:%02d:%02d]",clockAlias,atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->on[0],
                                dailyTO[atoi(iot.inline_param[0])]->on[1],
                                dailyTO[atoi(iot.inline_param[0])]->on[2]);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "off_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->off[0]=atoi(iot.inline_param[2]); // hour
                        dailyTO[atoi(iot.inline_param[0])]->off[1]=atoi(iot.inline_param[3]); // minute
                        dailyTO[atoi(iot.inline_param[0])]->off[2]=atoi(iot.inline_param[4]); // seconds

                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])],atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Clock[#%d] [OFF] updated %02d:%02d:%02d",clockAlias, atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->off[0],
                                dailyTO[atoi(iot.inline_param[0])]->off[1],
                                dailyTO[atoi(iot.inline_param[0])]->off[2]);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "flag_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->flag=atoi(iot.inline_param[2]);
                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])],atoi(iot.inline_param[0]));
                        sprintf(msg, "%s: Clock[#%d] set to [%s]",clockAlias,
                                atoi(iot.inline_param[0]),atoi(iot.inline_param[2]) ? "ON" : "OFF");
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "status_daily_to") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {ON, %02d:%02d:%02d} {OFF, %02d:%02d:%02d} {Flag: %s}",
                                clockAlias,atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->on[0],
                                dailyTO[atoi(iot.inline_param[0])]->on[1],
                                dailyTO[atoi(iot.inline_param[0])]->on[2],
                                dailyTO[atoi(iot.inline_param[0])]->off[0],
                                dailyTO[atoi(iot.inline_param[0])]->off[1],
                                dailyTO[atoi(iot.inline_param[0])]->off[2],
                                dailyTO[atoi(iot.inline_param[0])]->flag ? "ON" : "OFF" );
                        iot.pub_msg(msg);
                }
        }
}

void setup() {
        // Serial.begin(9600);
        // Serial.println();
        // if (HARD_REBOOT) {
        //         EEPROM.begin(1024);
        // }
        // if (HARD_REBOOT) {
        //         // if (hReset_eeprom.val != 0) {
        //         // EEPROM.write(hReset_eeprom.val_cell,0);
        //         // EEPROM.put(0,100000);
        //         // EEPROM.commit();
        //         // delay(50);
        //         // Serial.print("value in flash: ");
        //         // Serial.println(EEPROM.get(0));
        //         // Serial.print("saved value: ");
        //         // Serial.println()
        //         // EEPROM.write(hReset_eeprom.wcount_cell,hReset_eeprom.wcount + 1);
        //
        //         // EEPROM.commit();
        //         // Serial.println("zeroing");
        //         // }
        // }

        // long boot_mil = millis();

        startGPIOs();
        quickPwrON();
        startIOTservices();

        for (int i=0; i<NUM_SWITCHES; i++) {
                check_dailyTO_inFlash(*dailyTO[i], i);
        }

        // Serial.print("value is: ");
        // Serial.print(hReset_eeprom.val);
        // Serial.print(" at cell num: ");
        // Serial.println(hReset_eeprom.val_cell);
        //
        // Serial.print("wearout is: ");
        // Serial.print(hReset_eeprom.wcount);
        // Serial.print(" at cell num: ");
        // Serial.println(hReset_eeprom.wcount_cell);





        // int a = millis()-boot_mil;
        // if (a < 2000) {
        //         delay(2000-a);
        //         Serial.print("Delayed: ");
        //         Serial.print(a);
        //         Serial.println("ms");
        // }
        // Serial.print("value is: ");
        // Serial.println(hReset_eeprom.val);
        // if (HARD_REBOOT) {
        //         if (hReset_eeprom.val != 0) {
        //                 EEPROM.write(hReset_eeprom.val_cell,0);
        //                 EEPROM.commit();
        //                 EEPROM.write(hReset_eeprom.wcount_cell,hReset_eeprom.wcount + 1);
        //                 EEPROM.commit();
        //                 // Serial.println("zeroing");
        //         }
        // }
}
void loop() {
        iot.looper();

        if (checkrebootState == true && USE_RESETKEEPER == true) {
                recoverReset();
        }

        for (int i=0; i<NUM_SWITCHES; i++) {
                if (USE_DAILY_TO == true) {
                        daily_timeouts_looper(*dailyTO[i],i);
                }
                if (USE_INPUTS == true) {
                        checkSwitch_Pressed(i);
                }
                timeOutLoop(i);
        }

        delay(100);
}
