
#include <myIOT.h>
#include <myJSON.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER              "Wemos_1.3"
#define USE_INPUTS       true
#define STATE_AT_BOOT    false // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO     true
#define IS_SONOFF        false

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      3*60 // mins for SW0
#define TIMEOUT_SW1      3*60 // mins
int clockOn_0 [2] = {8,0};
int clockOn_1 [2] = {18,0};

int clockOff_0[2] = {0,30};
int clockOff_1[2] = {22,0};


// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "tableLEDs"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "LEDStrips"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************



// ~~~~~~~ TimeOuts class ~~~~~~~~~
timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
#if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1",TIMEOUT_SW1);
timeOUT *TO[]={&timeOut_SW0,&timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[]={&timeOut_SW0};
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~ Use Daily Clock ~~~~
bool inDailyTO[] = {false, false};
#if USE_DAILY_TO
myJSON clock_inFlash("file0.json", true);
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and Statdes ~~~~
#if IS_SONOFF
#define RELAY1          12
#define RELAY2          5
#define INPUT1          14
#define INPUT2          0
#endif

#if !IS_SONOFF
#define RELAY1          D2
#define RELAY2          D7
#define INPUT1          D1
#define INPUT2          3
#endif

#define LEDpin          13
byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ ResetKeeper Vars ~~~~~~~
bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW

bool swState [NUM_SWITCHES];
bool last_swState [NUM_SWITCHES];
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
                        sprintf(msg2,"[%s]", msg1);
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
void checkSwitch_Pressed (byte sw){
        if (digitalRead(inputs[sw])==LOW) {
                delay(50);
                if (digitalRead(inputs[sw])==LOW) {
                        if (digitalRead(relays[sw])==RelayOn) {
                                TO[sw]->endNow();
                        }
                        else {
                                TO[sw]->restart_to();
                        }
                        delay(500);
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
        iot.start_services(ADD_MQTT_FUNC);
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
void quickPwrON(){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */
        for(int i=0; i<NUM_SWITCHES; i++) {
                if (TO[i]->savedTO) {
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
        if(iot.mqtt_detect_reset != 2) {
                badReboot = iot.mqtt_detect_reset;
                checkbadReboot = false;
                for (int i=0; i<NUM_SWITCHES; i++) {
                        if (TO[i]->updatedTO!=0) {    // coded TO was changed by user
                                TO[i]->inCodeTO = TO[i]->updatedTO;
                                sprintf(mqttmsg,"Switch[#%d]: get TimeOut from Flash",i);
                                iot.pub_err(mqttmsg);
                        }
                        if (badReboot == 0) {             // PowerOn - not a quickReboot
                            if (STATE_AT_BOOT == true ){  // define to be ON at boot
                                TO[i]->restart_to();
                              }
                        }
                        else {
                                TO[i]->begin(false);
                                iot.pub_err("--> Quick-Reset");
                        }
                }
        }
}
void timeOutLoop(){
        char msg_t[50], msg[50];

        if(iot.mqtt_detect_reset != 2) {
                for(int i=0; i<NUM_SWITCHES; i++) {
                        swState[i] = TO[i]->looper();
                        if (swState[i]!=last_swState[i]) { // change state (ON <-->OFF)
                                if (swState[i]==1) {    // swithc ON
                                        switchIt("TimeOut",i,1);
                                        if (IS_SONOFF) {
                                                digitalWrite(LEDpin,LOW);
                                        }
                                }
                                else{ // switch OFF
                                        switchIt("TimeOut",i,0);
                                        if (IS_SONOFF) {
                                                digitalWrite(LEDpin,HIGH);
                                        }
                                }
                        }
                        last_swState[i] = swState[i];
                }
        }
}
void daily_timeouts(int toff_vect[2],int ton_vect[2], byte i=0){
        char msg [50], msg2[50];
        time_t t=now();

        if (hour(t)==ton_vect[0] && minute(t)==ton_vect[1] && second(t)<2 && inDailyTO[i] == false) {
                int mins = toff_vect[1] - ton_vect[1];
                int delt_h = toff_vect[0] - ton_vect[0];

                int total_time = mins+ delt_h*60;
                if (total_time < 0){
                  total_time +=24*60;
                }

                TO[i]->setNewTimeout(total_time);
                TO[i]->convert_epoch2clock(now()+total_time*60,now(), msg2, msg);
                sprintf(msg, "Clock: Switch[#%d] [On] TimeOut [%s]", i,msg2);
                iot.pub_msg(msg);
                inDailyTO[i] = true;
        }
        else if (hour(t)==toff_vect[0] && minute(t)==toff_vect[1] && second(t)<2 && digitalRead(relays[i]) == RelayOn) {
                TO[i]->endNow();
                sprintf(msg, "Clock: Switch[#%d] [Off]", i);
                iot.pub_msg(msg);
                inDailyTO[i] = false;
        }
}
void check_dailyTO_inFlash(){
}
void addiotnalMQTT(char incoming_msg[50]) {
        char msg[150];
        char msg2[20];
        if      (strcmp(incoming_msg, "status") == 0) {
                for(int i=0; i<NUM_SWITCHES; i++) {
                        if(TO[i]->remain()>0) {
                                TO[i]->convert_epoch2clock(now()+TO[i]->remain(),now(), msg, msg2);
                                sprintf(msg2,"TimeOut[%s]", msg);
                        }
                        else{
                                sprintf(msg2,"");
                        }
                        sprintf(msg, "Status: Switch[#%d] [%s] %s",i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d], UseInputs[%d], DailyTO[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP, USE_INPUTS, USE_DAILY_TO);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
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
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+TO[atoi(iot.inline_param[0])]->remain(),now(), msg2, msg);
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
        }
}

void setup() {
        startGPIOs();
        quickPwrON();
        startIOTservices();
}
void loop() {
        iot.looper();
        timeOutLoop();

        if (checkbadReboot == true && USE_RESETKEEPER == true) {
                recoverReset();
        }
        if (USE_DAILY_TO == true) {
                for (int i=0; i<NUM_SWITCHES; i++) {
                        daily_timeouts(clockOff_0, clockOn_0,i);
                }
        }
        if (USE_INPUTS == true) {
                checkSwitch_Pressed(0);
        }

        delay(100);
}