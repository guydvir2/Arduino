
/*
   ESP8266 Alarm monitoring system, by guyDvir Apr.2019

   Pupose of this code is to monitor, notify and change syste, states of an existing
   Alarm system ( currently using PIMA alarm system incl. RF detectors) has the
   following hardware:
   1) Main Controller AKA " the brain"
   2) KeyPad for entering user codes and change syste states.
   3) "the brain" has relay inputs knows as external key operation:
 */



#include <myIOT.h>
#include <Arduino.h>

// ********** Names + Strings  ***********
#define Telegram_Nick "Alarm"                         // belongs to TELEGRAM

// ~~~~~~~ MQTT Topics ~~~~~~                        // belonga rto myIOT
#define DEVICE_TOPIC "AlarmMonitor"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~


// ********** Sketch Services  ***********
#define VER              "NODEMCU_2.0"
#define USE_NOTIFY_TELE  true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true // Serial Monitor
#define USE_WDT          true  // watchDog resets
#define USE_OTA          true  // OTA updates
#define USE_RESETKEEPER  false // detect quick reboot and real reboots
#define USE_FAILNTP      true  // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~ HW Pins and States ~~~~
// GPIO Pins for ESP8266
#define INPUT1   4          // input (System State) systemState_alarm_Pin
#define INPUT2   5          // input (System State)   systemState_armed_Pin
#define OUTPUT1  12         //  output (Set system)  armed_Home
#define OUTPUT2  14         //  Output (Set system)  Armed_Away
//##########################

byte relays[]       = {OUTPUT1, OUTPUT2};
byte inputs[]       = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn HIGH
#define SwitchOn LOW

// GPIO status flags
bool indication_ARMED_lastState   = true;
bool indication_ALARMED_lastState = true;

const int systemPause = 2000; // milli-seconds, delay to system react
const int deBounceInt = 50;

// #################################  END CORE #################################



//  ######################### ADDITIONAL SERVICES ##############################

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE

myTelegram teleNotify(BOT_TOKEN, CHAT_ID);

#endif

// #############################################################################



//  ############################## STRART CORE #################################

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
        for (int i = 0; i<2; i++) {
                pinMode(relays[i],OUTPUT);
                pinMode(inputs[i],INPUT_PULLUP);
        }
}


// ~~~~ MQTT Commands ~~~~~
void addiotnalMQTT(char *incoming_msg){
        char msg[100];

        if (strcmp(incoming_msg, "status") == 0) {
                giveStatus(msg);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "armed_home") == 0 || strcmp(incoming_msg, "armed_away") == 0 || strcmp(incoming_msg, "disarmed") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], Telegram[%d]", VER,iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_NOTIFY_TELE);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: input1[%d] input2[%d], Relay: output_home[%d] output_full[%d]", INPUT1, INPUT2, OUTPUT1, OUTPUT2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "reset") == 0 ) {
                allReset();
                iot.sendReset("Reset via MQTT");
        }
        else if (strcmp(incoming_msg, "all_off") == 0 ) {
                all_off("MQTT");

        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help, pins]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [armed_home, armed_away, disarmed, reset, all_off]");
                iot.pub_msg(msg);
        }
}
void giveStatus(char *state){
        char t3 [50];

        sprintf(t3, "");
        // relays state
        if (digitalRead(OUTPUT1) == RelayOn && digitalRead(OUTPUT2) == RelayOn) {
                strcat(t3, "Status: invalid [Armed] and [Away] State");
        }
        else if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn && digitalRead(INPUT1) == SwitchOn) {
                strcat(t3, "Status: Manual [Armed]");
        }
        else if (digitalRead(OUTPUT1) == RelayOn && digitalRead(OUTPUT2) == !RelayOn && digitalRead(INPUT1) == SwitchOn) {
                strcat(t3, "Status: [Code] [Home Armed]");
        }
        else if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == RelayOn && digitalRead(INPUT1) == SwitchOn) {
                strcat(t3, "Status: [Code] [Armed Away]");
        }
        else if (digitalRead(INPUT1) == SwitchOn && digitalRead(INPUT2)== SwitchOn) {
                strcat(t3, "Status: [Alarm]");
        }
        else if (digitalRead(INPUT1) == !SwitchOn && digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn) {
                strcat(t3, "Status: [disarmed]");
        }
        else {
                strcat(t3, "Status: [notDefined]");

        }
        sprintf(state,"%s",t3);
}
void all_off(char *from){
        char t[50];
        allOff();

        sprintf(t,"All OFF: Received from %s",from);
        iot.pub_msg(t);
}

// ~~~~ maintability ~~~~~~
void allOff() {
        for (int i = 0; i<2; i++) {
                digitalWrite(relays[i],!RelayOn);
        }
        delay(systemPause);
}
void allReset() {
        for (int i = 0; i<2; i++) {
                digitalWrite(relays[i], RelayOn);
                delay(systemPause);
                digitalWrite(relays[i], !RelayOn);
        }
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool states[2];

        if (strcmp(dir, "armed_home") == 0 ) {
                if (digitalRead(OUTPUT1) == !RelayOn) { // verify it is not in desired state already
                        if ( digitalRead(OUTPUT2) == RelayOn) { // in armed away state
                                digitalWrite(OUTPUT2, !RelayOn);
                                iot.pub_msg("System is set to [Disarmed] [Away] using [Code]");
                                delay(systemPause);
                        }

                        digitalWrite(OUTPUT1, RelayOn);         // Now switch to armed_home
                        delay(systemPause);

                        if (digitalRead(INPUT1)==SwitchOn) {
                                iot.pub_msg("System is set to [Armed] [Home] using [Code]");
                                iot.pub_state("armed_home");
                        }
                        else {
                                allOff();
                                iot.pub_msg("failed to [Armed] [Home]");
                        }
                }
                else {
                        iot.pub_msg("already in [Armed] [Home]");
                }
        }
        else if (strcmp(dir, "armed_away") == 0) {
                if ( digitalRead(OUTPUT2) == !RelayOn) {
                        if ( digitalRead(OUTPUT1) == RelayOn) { // armed home
                                digitalWrite(OUTPUT1, !RelayOn);
                                iot.pub_msg("System is set to [Disarmed] [Home] using [Code]");
                                delay(systemPause);
                        }

                        digitalWrite(OUTPUT2, RelayOn); // now switch to Away
                        delay(systemPause);

                        if (digitalRead(INPUT1)==SwitchOn) {
                                iot.pub_msg("System is set to [Armed] [Away] using [Code]");
                                iot.pub_state("armed_away");
                        }
                        else {
                                allOff();
                                iot.pub_msg("failed to [Armed] [Away]");
                        }
                }
                else {
                        iot.pub_msg("already in [Armed] [Away]");
                }
        }
        else if (strcmp(dir, "disarmed") == 0) {
                if (indication_ARMED_lastState == SwitchOn) { // system is armed
                        if (digitalRead(OUTPUT2)==RelayOn || digitalRead(OUTPUT1)==RelayOn) { // case of Remote operation
                                allOff();
                        }
                        else { // case of manual operation
                                // initiate any arm state in order to disarm
                                digitalWrite(OUTPUT1, RelayOn);
                                delay(systemPause);
                                allOff();
                        }
                        if (digitalRead(INPUT1)==!SwitchOn && digitalRead(OUTPUT2)==!RelayOn && digitalRead(OUTPUT1)==!RelayOn) {
                                iot.pub_msg("System is set to [Disarmed]");
                                send_telegramAlert("System is set to [Disarmed]");
                                iot.pub_state("disarmed");
                        }
                        else {
                                iot.pub_msg("failed to [Disarmed]");
                        }
                }
        }
}
void check_systemState_armed() { // System OUTPUT 1: arm_state
        if (digitalRead(INPUT1) != indication_ARMED_lastState) {
                delay(deBounceInt);
                if (digitalRead(INPUT1) != indication_ARMED_lastState) {
                        delay(systemPause);

                        if (digitalRead(INPUT1) == SwitchOn) { // system is set to armed
                                if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn) {
                                        iot.pub_msg("system is set to [Armed] using [KeyPad] ");
                                        iot.pub_state("pending");
                                }
                                else if (digitalRead(OUTPUT2) == RelayOn || digitalRead(OUTPUT1) == RelayOn) {
                                        iot.pub_msg("system is set to [Armed] using [Code]");
                                }
                        }

                        else { // system Disarmed
                                if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn) {
                                        iot.pub_msg("system is set to [Disarmed] using [KeyPad]");
                                        iot.pub_state("disarmed");
                                }
                                else {
                                        allOff();
                                        if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn) {
                                                iot.pub_msg("system is set to [Disarmed] using [Code]");
                                                iot.pub_state("disarmed");
                                        }
                                        else {
                                                iot.pub_msg("failed to [Disarmed]");
                                                iot.sendReset("failed to Disarm");
                                        }
                                }
                        }
                        indication_ARMED_lastState = digitalRead(INPUT1);
                }
        }
}
void check_systemState_alarming() { // // System OUTPUT 2: alarm_state
        if (digitalRead(INPUT2) != indication_ALARMED_lastState) {
                delay(deBounceInt);
                if (digitalRead(INPUT2) != indication_ALARMED_lastState) {
                        delay(systemPause);
                        // alarm set off
                        if (digitalRead(INPUT2) == SwitchOn) {
                                iot.pub_msg("System is [Alarming!]");
                                iot.pub_state("triggered");
                                send_telegramAlert("in [Alarm]");
                        }
                        // alarm ended
                        else if (digitalRead(INPUT2) == !SwitchOn) {
                                iot.pub_msg("System stopped [Alarming]");
                                send_telegramAlert("[Alarm] ended");
                                if (digitalRead(INPUT1)==SwitchOn) {
                                        if (digitalRead(OUTPUT2)==RelayOn) {
                                                iot.pub_state("armed_away");
                                        }
                                        else if (digitalRead(OUTPUT1)==RelayOn) {
                                                iot.pub_state("armed_home");
                                        }
                                        else{
                                                iot.pub_state("pending");
                                        }
                                }
                                else{
                                        iot.pub_state("disarmed");
                                }
                        }
                        indication_ALARMED_lastState = digitalRead(INPUT2);
                }
        }
}


//  ######################### ADDITIONAL SERVICES ##############################
// ~~~~~~~~~~~ Telegram Notify ~~~~~~~
#if USE_NOTIFY_TELE
void telecmds(String in_msg, String from, String chat_id, char snd_msg[150]) {
        String command_set[] = {"whois_online", "status", "reset", "home", "away", "disarm", "whoami","help"};
        byte num_commands = sizeof(command_set)/sizeof(command_set[0]);
        String comp_command[num_commands];
        char prefix[50];
        char t1[150];

        sprintf(prefix,"/%s_",Telegram_Nick);
        for (int i=0; i < num_commands; i++) {
                comp_command[i].concat(prefix);
                comp_command[i] += command_set[i];
        }

        if(in_msg=="/whois_online") {
                sprintf(snd_msg,"~%s~",Telegram_Nick);
        }  // all devices should answer this
        else if (in_msg==comp_command[1]) {
                giveStatus(t1);
                sprintf(snd_msg,"~%s~ %s",DEVICE_TOPIC, t1);
        } // status
        else if (in_msg==comp_command[2]) {
                iot.sendReset("Telegram");
        } // reset
        else if (in_msg==comp_command[3]) {
          switchIt("Telegram", "armed_home");
          sprintf(snd_msg,"~%s~ armed_home", DEVICE_TOPIC);
        } //home
        else if (in_msg==comp_command[4]) {
          switchIt("Telegram", "armed_away");
                sprintf(snd_msg,"~%s~ armed_away",DEVICE_TOPIC);
        } // away
        else if (in_msg==comp_command[5]) {
          switchIt("Telegram", "disarmed");
                sprintf(snd_msg,"~%s~ disarmed",DEVICE_TOPIC);
        } // away
        else if (in_msg==comp_command[6]) {
                sprintf(snd_msg,"~%s~ is %s", DEVICE_TOPIC,Telegram_Nick);
        } // whoami
        else if (in_msg==comp_command[7]) {
                char t[50];
                sprintf(snd_msg,"~%s~ Available commands:\n", DEVICE_TOPIC);
                for (int i=0; i<num_commands; i++) {
                        command_set[i].toCharArray(t,30);
                        sprintf(t1,"%s\n",t);
                        strcat(snd_msg,t1);
                }

        } // help
}
#endif
void send_telegramAlert(char *msg){
        char outmsg[50];
        sprintf(outmsg,"~%s~: %s", DEVICE_TOPIC,msg);
        #if USE_NOTIFY_TELE
        teleNotify.send_msg(outmsg);
        #endif
}
// ########################### END ADDITIONAL SERVICE ##########################


void setup() {
        startGPIOs();
        startIOTservices();

            #if USE_NOTIFY_TELE
        teleNotify.begin(telecmds);
        send_telegramAlert("Boot");
            #endif
}
void loop() {
        iot.looper();

            #if USE_NOTIFY_TELE
        teleNotify.looper();
            #endif

        check_systemState_armed();
        check_systemState_alarming();

        delay(200);
}
