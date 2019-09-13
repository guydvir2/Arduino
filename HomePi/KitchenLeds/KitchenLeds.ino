#include <myIOT.h>
#include <myJSON.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <EEPROM.h>

// ********** Sketch Services  ***********
#define VER              "Wemos.3.5"
#define USE_INPUTS       true
#define STATE_AT_BOOT    false
#define USE_DAILY_TO     true
#define IS_SONOFF        true
#define HARD_REBOOT      true
#define USE_IR_REMOTE    true

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      3*60 // mins for SW0
#define TIMEOUT_SW1      3*60 // mins
// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "KitchenLEDs"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "LEDStrips"
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

char *clock_fields[] = {"ontime", "off_time", "flag","use_inFl_vals"};
int items_each_array[3] = {3,3,1};
char *clockAlias = "DailyClock";
struct dTO {
        int on[3];
        int off[3];
        bool flag;
        bool onNow;
        bool useFlash;
};
dTO defaultVals = {{0,0,0},{0,0,59},0,0,0};
dTO dailyTO_0   = {{19,30,0},{02,30,0},1,0,0};
dTO dailyTO_1   = {{20,00,0},{22,0,0},1,0,0};
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
#define RELAY1          D2 // 12 for Sonoff D2 for wemos mini
#define RELAY2          5
#define INPUT1          9
#define INPUT2          3

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

//~~~~~~~Run IR Remote ~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

#define IR_SENSOR_PIN D5

const uint16_t kRecvPin        = IR_SENSOR_PIN;
const uint32_t kBaudRate       = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value        = 0;

IRrecv irrecv(kRecvPin);
decode_results results;

void recvIRinputs() {
#if USE_IR_REMOTE
        char msg[50];

        if (irrecv.decode(&results)) {

                if (results.value == 0XFFFFFFFF)
                        results.value = key_value;
                char msg[50];

                switch (results.value) {
                case 0xFFA25D:
                        //Serial.println("CH-");
                        break;
                case 0xFF629D:
                        //Serial.println("CH");
                        iot.sendReset("RemoteControl");
                        break;
                case 0xFFE21D:
                        //Serial.println("CH+");
                        break;
                case 0xFF22DD:
                        //Serial.println("|<<");
                        break;
                case 0xFF02FD:
                        //Serial.println(">>|");
                        break;
                case 0xFFC23D:
                        //Serial.println(">|");
                        break;
                case 0xFFE01F:
                        //Serial.println("-");
                        TO[0]->endNow();
                        sprintf(msg, "TimeOut: IRremote[Abort]");
                        iot.pub_msg(msg);
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        TO[0]->restart_to();
                        sprintf(msg, "TimeOut: IRremote[Restart]");
                        iot.pub_msg(msg);
                        break;
                case 0xFF906F:
                        //Serial.println("EQ");
                        break;
                case 0xFF6897:
                        //Serial.println("0");
                        break;
                case 0xFF9867:
                        //Serial.println("100+");
                        break;
                case 0xFFB04F:
                        //Serial.println("200+");
                        break;
                case 0xFF30CF:
                        // blinker_state = !blinker_state;
                        // sprintf(msg, "IRremote: [Blinker], [%s]", blinker_state ? "ON" : "OFF");
                        // iot.pub_msg(msg);
                        break;
                case 0xFF18E7:
                        // strobe_state = !strobe_state;
                        // sprintf(msg, "Switch: [Strobe], [%s]", strobe_state ? "ON" : "OFF");
                        // iot.pub_msg(msg);
                        break;
                case 0xFF7A85:
                        break;
                case 0xFF10EF:
                        //change_color(4);
                        break;
                case 0xFF38C7:
                        //change_color(5);
                        break;
                case 0xFF5AA5:
                        //Serial.println("6");
                        //                        change_color(6);
                        break;
                case 0xFF42BD:
                        //Serial.println("7");
                        //                        change_color(7);
                        break;
                case 0xFF4AB5:
                        //Serial.println("8");
                        //                        change_color(8);
                        break;
                case 0xFF52AD:
                        //Serial.println("9");
                        //                        change_color(9);
                        break;
                }
                key_value = results.value;
                irrecv.resume();
        }
#endif
}
void start_IR() {
#if USE_IR_REMOTE
#if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
        irrecv.enableIRIn(); // Start the receiver
#endif
}

#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
void check_hardReboot(byte i=1, byte threshold = 2){
        hReset_eeprom.jump = EEPROM.read(0);

        hReset_eeprom.val_cell    = hReset_eeprom.jump + i;
        // hReset_eeprom.wcount_cell = hReset_eeprom.val_cell + 1;

        hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);
        // hReset_eeprom.wcount = EEPROM.read(hReset_eeprom.wcount_cell);

        if (hReset_eeprom.val < threshold) {
                EEPROM.write(hReset_eeprom.val_cell,hReset_eeprom.val+1);
                EEPROM.commit();
                hReset_eeprom.hBoot = false;
        }
        else {
                hReset_eeprom.hBoot = true;
        }
}
void quickPwrON(){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */

        /*
         # conditions in for loop:
           1) Has more time to go in TO
           2) STATE_AT_BOOT defines to be ON at bootTime
           3) eeprom Reset counter forces to be ON_AT_BOOT
         */

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
                        if (rebootState == 0 ) { //}|| ) {  // PowerOn - not a quickReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> NormalBoot");
                        }
                        else if (hReset_eeprom.hBoot ) { // using HardReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> ForcedBoot");
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
                sprintf(temp,"%s_%d",clock_fields[3], x);
                dailyTO_inFlash.getValue(temp, retVal);
                if (retVal) { //only if flag is to read values from flash
                        for(int m=0; m<sizeof(clock_fields)/sizeof(clock_fields[0]); m++) {
                                sprintf(temp,"%s_%d",clock_fields[m], x);
                                if (m == 0 || m == 1) { // clock fileds only -- on or off
                                        for(int i=0; i<items_each_array[m]; i++) {
                                                dailyTO_inFlash.getArrayVal(temp,i,retVal);
                                                if (retVal >=0) {
                                                        if((i==0 && retVal<=23)||(i>0 && retVal<=59)) { //valid time
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
                                else {       // for flag value
                                        dailyTO_inFlash.getValue(temp, retVal);
                                        if (retVal == 0 || retVal == 1) { //flag on or off
                                                if(m==2) {
                                                        dailyTO.flag = retVal;
                                                }
                                                else if (m==3) {
                                                        dailyTO.useFlash = retVal;
                                                }
                                        }
                                        else {
                                                dailyTO_inFlash.setValue(temp,0);
                                        }
                                }
                        }

                }
                else{ // create NULL values
                        store_dailyTO_inFlash(defaultVals,x);
                }
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
                else if (m==3) {
                        dailyTO_inFlash.setValue(temp,dailyTO.useFlash);

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
                sprintf(msg, "ver #2: DailyTO[%d], UseInputs[%d], STATE_AT_BOOT[%d]",USE_DAILY_TO, USE_INPUTS, STATE_AT_BOOT);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to, status_TO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #4 - [off_daily_to, on_daily_to, flag_daily_to, useflash_daily_to]");
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

                        sprintf(msg, "%s: Switch[#%d] [ON] updated [%02d:%02d:%02d]",clockAlias,atoi(iot.inline_param[0]),
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

                        sprintf(msg, "%s: Switch[#%d] [OFF] updated [%02d:%02d:%02d]",clockAlias, atoi(iot.inline_param[0]),
                                dailyTO[atoi(iot.inline_param[0])]->off[0],
                                dailyTO[atoi(iot.inline_param[0])]->off[1],
                                dailyTO[atoi(iot.inline_param[0])]->off[2]);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "flag_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->flag=atoi(iot.inline_param[2]);
                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])],atoi(iot.inline_param[0]));
                        sprintf(msg, "%s: Switch[#%d] set to [%s]",clockAlias,
                                atoi(iot.inline_param[0]),atoi(iot.inline_param[2]) ? "ON" : "OFF");
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "useflash_daily_to") == 0) {
                        dailyTO[atoi(iot.inline_param[0])]->useFlash=atoi(iot.inline_param[2]);
                        store_dailyTO_inFlash(*dailyTO[atoi(iot.inline_param[0])],atoi(iot.inline_param[0]));
                        sprintf(msg, "%s: Switch[#%d] using [%s] values",clockAlias,
                                atoi(iot.inline_param[0]),atoi(iot.inline_param[2]) ? "Flash" : "Code");
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
        if (HARD_REBOOT) {
                EEPROM.begin(1024);
                check_hardReboot();
        }

        startGPIOs();
        start_IR();
        quickPwrON();
        startIOTservices();

        for (int i=0; i<NUM_SWITCHES; i++) {
                check_dailyTO_inFlash(*dailyTO[i], i);
        }

        if (HARD_REBOOT) {
                EEPROM.write(hReset_eeprom.val_cell,0);
                EEPROM.commit();
        }

}
void loop() {
        iot.looper();
        recvIRinputs(); // IR signals


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
