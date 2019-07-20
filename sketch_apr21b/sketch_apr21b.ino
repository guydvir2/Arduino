#include <myIOT.h>
#include <Arduino.h>
#include <TimeLib.h>

#define VER  "NodeMCU_2.2"

// ********** myIOT ***********

//~~~~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "basic"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "no_group"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************



// ******** timeOUT  *********
#define TIMEOUT_SW0      1 // mins
#define TIMEOUT_SW1      2 // mins

int CLOCK_ON[2] ={12,53};
int CLOCK_OFF[2]={12,55};

timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
timeOUT timeOut_SW1("SW1",TIMEOUT_SW1);
timeOUT *TO[]={&timeOut_SW0,&timeOut_SW1};
// ***************************

#define USE_BOUNCE_DEBUG false
#define USE_INPUTS       false
#define USE_DAILY_TO     true


// ~~~~~~~~ HW Pins  ~~~~~~~~
#define NUM_SWITCHES    2
#define RELAY1          5
#define RELAY2          12
#define INPUT1          9
#define INPUT2          3

byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW

bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;
bool swState [NUM_SWITCHES];
bool last_swState [NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~


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
}
void quickPwrON(){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */
        int temp=0;
        for(int i=0; i<NUM_SWITCHES; i++) {
                TO[i]->endTimeOUT_inFlash.getValue(temp);
                if (temp>0) {
                        digitalWrite(1, HIGH);
                }
                else{
                        digitalWrite(2, LOW);
                }
        }
}
void switchIt(char *type, int sw_num, char *dir) {
        char mqttmsg[150];
        char states[50];
        char tempstr[50];

        if (sw_num < NUM_SWITCHES && sw_num >= 0) {
                if (strcmp(dir, "on") == 0 || strcmp(dir, "off") == 0) {
                        digitalWrite(relays[sw_num], RelayOn);
                        if (strcmp(type, "TimeOut") == 0) {
                                char msg[20];
                                char msg_t[20];

                                if (strcmp(dir, "on") == 0) {
                                        TO[sw_num]->convert_epoch2clock(now()+TO[sw_num]->remain(),now(), msg_t, msg);
                                        sprintf(mqttmsg, "%s: Switch[#%d] [%s] [%s]", type, sw_num, dir,msg_t);
                                }
                                else{
                                        sprintf(mqttmsg, "%s: Switch[#%d] [%s]", type, sw_num, dir);

                                }
                        }
                        else {
                                sprintf(mqttmsg, "%s: Switch[#%d] [%s]", type, sw_num, dir);
                        }
                }
                iot.pub_msg(mqttmsg);

                sprintf(states,"");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "On" : "Off");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
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
        if(iot.mqtt_detect_reset != 2) {
                badReboot = iot.mqtt_detect_reset;
                checkbadReboot = false;
                int ext_timeout;
                for (int i=0; i<NUM_SWITCHES; i++) {
                        if (badReboot == 0) {                  // PowerOn - not a quickReboot
                                if (TO[i]->updatedTimeOUT_inFlash.getValue(ext_timeout)) { // Read Flash
                                        if (ext_timeout!=0) {  // coded TO was changed by user
                                                TO[i]->inCode_timeout_value = ext_timeout;
                                                iot.pub_err("TimeOut updated by User");
                                        }
                                        TO[i]->restart_to();
                                }
                                else{   // fail to read FLASH value
                                        TO[i]->updatedTimeOUT_inFlash.setValue(0);
                                        iot.pub_err("Fail reading updated TimeOut");
                                }
                        }
                        else {                       // badReboot detected
                                TO[i]->begin(false); // badreboot detected - don't restart TO if already ended
                                iot.pub_err("badReboot");
                        }
                }
        }
}
void timeOutLoop(){
        char msg_t[50];
        char msg[50];

        char *states[2]={"off","on"};

        if(iot.mqtt_detect_reset != 2) {
                for(int i=0; i<NUM_SWITCHES; i++) {
                        swState[i] = TO[i]->looper();
                        if (swState[i]!=last_swState[i]) { // change state (ON <-->OFF)
                                if (swState[i]==1) {    // swithc ON
                                        switchIt("TimeOut", i, states[1]);
                                }
                                else{ // switch OFF
                                        switchIt("TimeOut",i,states[0]);
                                }
                        }
                        last_swState[i] = swState[i];
                }
        }
}
void checkSwitch_Pressed() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (digitalRead(inputs[i]) != inputs_lastState[i]) {
                        delay(50);
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
void daily_timeouts(int toff_vect[2],int ton_vect[2], byte i=0){
        time_t t=now();
        if (hour(t)==toff_vect[0] && minute(t)==toff_vect[1] && second(t)<2) {
                if (digitalRead(relays[i])==RelayOn) {
                        switchIt("Clock",i,"off");
                }
        }
        if (hour(t)==ton_vect[0] && minute(t)==ton_vect[1] && second(t)<2) {
                if (digitalRead(relays[i])==!RelayOn) {
                        switchIt("Clock",i,"on");
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
                                sprintf(msg2,", TimeOut[%s]", msg);
                        }
                        else{
                                sprintf(msg2,"");
                        }
                        sprintf(msg, "Status: Switch[#%d] [%s] %s",i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d],  ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
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

                if (strcmp(iot.inline_param[1],"on") == 0 || strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), iot.inline_param[1]);
                }
                else if(strcmp(iot.inline_param[1],"timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+atoi(iot.inline_param[2]),now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[%d] new TimeOut Added %s", atoi(iot.inline_param[0]),msg2);
                        iot.pub_msg(msg);
                }
                else if(strcmp(iot.inline_param[1],"updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg, "TimeOut: Switch[%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg);
                        delay(1000);
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
                        iot.sendReset("Restore");
                }
        }
}
void setup() {
        startGPIOs();
        quickPwrON();

        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);

}
void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        timeOutLoop();

        if (checkbadReboot == true) {
                recoverReset();
        }
        if (USE_DAILY_TO == true) {
                daily_timeouts(CLOCK_OFF, CLOCK_ON,0);
        }
        if (USE_INPUTS == true) {
                checkSwitch_Pressed();
        }

        delay(100);
}
