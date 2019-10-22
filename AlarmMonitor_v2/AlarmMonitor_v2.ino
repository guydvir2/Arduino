
/*
   ESP8266 Alarm monitoring system, by guyDvir Apr.2019

   Pupose of this code is to monitor, notify and change syste, states of an existing
   Alarm system ( currently using PIMA alarm system incl. RF detectors) has the
   following hardware:
   1) Main Controller AKA " the brain"
   2) KeyPad for entering user codes and change syste states.
   3) "the brain" has relay inputs knows as external key operation:
 */
// Previous version 2.1, iot1.43

/*

 |  INPUT1  |   INPUT2    |   OUTPUT1  |  OUTPUT2   |      Sys      |
 | Arm Indic| Alarm indic | Home Relay | Away Relay |     State     |
 +==========+=============+============+============+===============+
 |    1     |      1      |      0     |     0      | disarmed All  |
 +----------+-------------+------------+------------+---------------+
 |    0     |      1      |      1     |     0      | Home Arm code |
 +----------+-------------+------------+------------+---------------+
 |    0     |      1      |      0     |     1      | Away Arm code |
 +----------+-------------+------------+------------+---------------+
 |    0     |      1      |      0     |     0      |Home/awa keypad|
 +----------+-------------+------------+------------+---------------+
 |    0     |      0      |      0/1   |     0/1    |     Alarm     |
 +----------+-------------+------------+------------+---------------+

 */


#include <myIOT.h>
#include <Arduino.h>

// ********** Names + Strings  ***********
#define Telegram_Nick "Alarm"                         // belongs to TELEGRAM

// ~~~~~~~ MQTT Topics ~~~~~~                        // belonga rto myIOT
#define DEVICE_TOPIC "alarmMonitor"
#define MQTT_PREFIX  "HomePi/Dvir"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~


// ********** Sketch Services  ***********
#define VER              "NODEMCU_3.1"
#define USE_NOTIFY_TELE  true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false // Serial Monitor
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
#define INPUT1   4          //  Indication system is Armed
#define INPUT2   5          //  Indication system is Alarmed
#define OUTPUT1  12         //   (Set system)  armed_Home
#define OUTPUT2  14         //   (Set system)  Armed_Away

#define RelayOn HIGH
#define SwitchOn LOW

byte relays[]       = {OUTPUT1, OUTPUT2};
byte inputs[]       = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

// ~~~~~~~~~~~~~~ Using Fvars to limit code ~~~~~~~~~~~~~~
FVars useTelegram("useT");       // Start or Stop sending Telegram Notifications
FVars monitorOnly("Mon_Only");   // Start or Stop Monitor-Only Mode ( recieving status without having any ability to chage Hardware state  )
int useT;
int monOnly;
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
        else if (strcmp(incoming_msg, "clear") == 0 ) {
                allReset();
                iot.sendReset("Reset via MQTT");
        }
        else if (strcmp(incoming_msg, "all_off") == 0 ) {
                all_off("MQTT");

        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help, pins]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [armed_home, armed_away, disarmed, clear, all_off, debug]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "debug") == 0) {
                sprintf(msg,"INPUT1 is [%d], INPUT2 is [%d], OUTPUT1 is [%d], OUTPUT2 is [%d]",digitalRead(INPUT1), digitalRead(INPUT2), digitalRead(OUTPUT1),digitalRead(OUTPUT2));
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
        if (monOnly == 0) {
                for (int i = 0; i<2; i++) {
                        digitalWrite(relays[i],!RelayOn);
                }
                delay(systemPause);
        }
}
void allReset() {
        if (monOnly == 0) {
                for (int i = 0; i<2; i++) {
                        digitalWrite(relays[i], RelayOn);
                        delay(systemPause);
                        digitalWrite(relays[i], !RelayOn);
                }
        }
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool armed_code;
        if (monOnly == 0) {
                if (strcmp(dir, "armed_home") == 0 ) {
                        if (digitalRead(OUTPUT1) == !RelayOn) { // verify it is not in desired state already
                                if ( digitalRead(OUTPUT2) == RelayOn) { // in armed away state
                                        digitalWrite(OUTPUT2, !RelayOn);
                                        iot.pub_msg("System change: [Disarmed] [Away] using [Code]");
                                        delay(systemPause);
                                }

                                digitalWrite(OUTPUT1, RelayOn); // Now switch to armed_home
                                delay(systemPause);

                                if (digitalRead(INPUT1)==SwitchOn) {
                                        iot.pub_msg("System change: [Armed] [Home] using [Code]");
                                        iot.pub_state("armed_home");
                                }
                                else {
                                        allOff();
                                        iot.pub_msg("System change: failed to [Armed] [Home]");
                                }
                        }
                        else {
                                iot.pub_msg("System change: already in [Armed] [Home]");
                        }
                }
                else if (strcmp(dir, "armed_away") == 0) {
                        if ( digitalRead(OUTPUT2) == !RelayOn) {
                                if ( digitalRead(OUTPUT1) == RelayOn) { // armed home
                                        digitalWrite(OUTPUT1, !RelayOn);
                                        iot.pub_msg("System change: [Disarmed] [Home] using [Code]");
                                        delay(systemPause);
                                }

                                digitalWrite(OUTPUT2, RelayOn); // now switch to Away
                                delay(systemPause);

                                if (digitalRead(INPUT1)==SwitchOn) {
                                        iot.pub_msg("System change: [Armed] [Away] using [Code]");
                                        iot.pub_state("armed_away");
                                }
                                else {
                                        allOff();
                                        iot.pub_msg("System change: failed to [Armed] [Away]");
                                }
                        }
                        else {
                                iot.pub_msg("System change: already in [Armed] [Away]");
                        }
                }
                else if (strcmp(dir, "disarmed") == 0) {
                        if (indication_ARMED_lastState == SwitchOn) { // indicatio n system is armed
                                if (digitalRead(OUTPUT2)==RelayOn || digitalRead(OUTPUT1)==RelayOn) { // case A: armed using code
                                        allOff();
                                        armed_code = true;
                                        delay(systemPause);
                                }
                                else { // case B: armed using keyPad
                                        // initiate any arm state in order to disarm
                                        digitalWrite(OUTPUT1, RelayOn);
                                        delay(systemPause/2);    // Time for system to react to fake state change
                                        allOff();
                                        armed_code = false;
                                        delay(systemPause/2);
                                }
                                if (digitalRead(INPUT1) != SwitchOn ) {//&& digitalRead(OUTPUT2) != RelayOn && digitalRead(OUTPUT1) != RelayOn) {
                                        sprintf(mqttmsg,"System change: [Disarmed] using [Code]. Was [Armed] using [%s]", armed_code ? "Code" : "KeyPad");
                                        iot.pub_msg(mqttmsg);
                                        iot.pub_state("disarmed");
                                }
                                else {
                                        sprintf(mqttmsg,"INPUT1 is [%d], INPUT2 is [%d], OUTPUT1 is [%d], OUTPUT2 is [%d]",digitalRead(INPUT1), digitalRead(INPUT2), digitalRead(OUTPUT1),digitalRead(OUTPUT2));
                                        // iot.pub_msg("System change: failed to [Disarmed]");
                                        iot.pub_msg(mqttmsg);
                                }
                        }
                }
        }
}
void check_systemState_armed() { // System OUTPUT 1: arm_state
        if (digitalRead(INPUT1) != indication_ARMED_lastState) {
                delay(deBounceInt);
                if (digitalRead(INPUT1) != indication_ARMED_lastState) {
                        delay(systemPause);

                        indication_ARMED_lastState = digitalRead(INPUT1);
                        if (indication_ARMED_lastState == SwitchOn) { // system is set to armed
                                if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn) {
                                        iot.pub_msg("System state: [Armed] using [KeyPad]");
                                        send_telegramAlert("[Armed]");

                                        iot.pub_state("pending");
                                }
                                else if (digitalRead(OUTPUT1) == RelayOn) {
                                        iot.pub_msg("System State: [Armed] [Home] using [Code]");
                                        send_telegramAlert("[Armed] [Home]");

                                }
                                else if (digitalRead(OUTPUT2) == RelayOn) {
                                        iot.pub_msg("System State: [Armed] [Away] using [Code]");
                                        send_telegramAlert("[Armed] [Away]");

                                }
                                else{
                                  iot.pub_err("Error Arming system");
                                }
                        }
                        else { // system detected a disarmed indication :
                                if (digitalRead(OUTPUT2)==RelayOn || digitalRead(OUTPUT1)==RelayOn) { // case A: armed using code, but disarmed by keypad
                                        allOff();
                                        // delay(systemPause); //seems tooo much
                                }
                                if (digitalRead(OUTPUT1) != RelayOn && digitalRead(OUTPUT2) != RelayOn) {
                                        iot.pub_msg("System State: [Disarmed]");
                                        send_telegramAlert("[Disarmed]");
                                        iot.pub_state("disarmed");
                                }
                        }
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
                                iot.pub_msg("[Alarm] is triggered");
                                iot.pub_state("triggered");
                                send_telegramAlert("[Alarm] is triggered");
                        }
                        // alarm ended
                        else if (digitalRead(INPUT2) == !SwitchOn) {
                                iot.pub_msg("[Alarm] stopped");
                                send_telegramAlert("[Alarm] stopped");
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
void telecmds(String in_msg, String from, String chat_id, char snd_msg[250]) {
        String command_set[] = {"whois_online", "status", "reset", "home", "away", "disarm",
                                "whoami","help","start_notifications", "stop_notifications",
                                "start_monOnly", "stop_monOnly"};
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
                sprintf(snd_msg,"~%s~ Reset command sent", DEVICE_TOPIC);
                iot.sendReset("Telegram");
        } // reset
        else if (in_msg==comp_command[3]) {
                switchIt("Telegram", "armed_home");
                sprintf(snd_msg,"~%s~ Armed-Home command sent", DEVICE_TOPIC);
        } //home
        else if (in_msg==comp_command[4]) {
                switchIt("Telegram", "armed_away");
                sprintf(snd_msg,"~%s~ Armed-Away command sent",DEVICE_TOPIC);
        } // away
        else if (in_msg==comp_command[5]) {
                switchIt("Telegram", "disarmed");
                sprintf(snd_msg,"~%s~ Disarmed command sent",DEVICE_TOPIC);
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
        else if (in_msg==comp_command[8]) {
                useTelegram.setValue(1);
                useT = 1;
                sprintf(snd_msg,"~%s~ Turn-On Telegram messages", DEVICE_TOPIC);
        } // turn on messages
        else if (in_msg==comp_command[9]) {
                useTelegram.setValue(0);
                useT = 0;
                sprintf(snd_msg,"~%s~ Turn-Off Telegram messages", DEVICE_TOPIC);
        } // turn on messages
        else if (in_msg==comp_command[10]) {
                monitorOnly.setValue(1);
                monOnly = 1;
                sprintf(snd_msg,"~%s~ set to Monitor-Only mode", DEVICE_TOPIC);
        } // start monitor only
        else if (in_msg==comp_command[11]) {
                monitorOnly.setValue(0);
                monOnly = 0;
                sprintf(snd_msg,"~%s~ stopped Monitor-Only mode", DEVICE_TOPIC);
        } // start monitor only
}
#endif
void send_telegramAlert(char *msg){
        char outmsg[50];
        sprintf(outmsg,"~%s~: %s", DEVICE_TOPIC,msg);
        #if USE_NOTIFY_TELE
        if (useT) {                             // stop or start receive messages using a flsh variable.
                teleNotify.send_msg(outmsg);
        }
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

        if(monitorOnly.getValue(monOnly)==0) { // not able to read flash value
                monitorOnly.setValue(0);       // store default
        }
        if(useTelegram.getValue(useT)==0) {    // not able to read flash value
                useTelegram.setValue(1);       // store default

        }

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
