#include <myIOT.h>
#include <Arduino.h>
#include <TimeLib.h>


// ********** Sketch Services  ***********
#define VER              "Wemos.Mini.2.71"
#define USE_BOUNCE_DEBUG false
#define USE_INPUTS       false
#define USE_DAILY_TO     true


// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      60*8 // mins for SW0
#define TIMEOUT_SW1      1 // mins
int CLOCK_ON[2] ={18,0};
int CLOCK_OFF[2]={8,0};


// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_IR_REMOTE    true
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
timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
# if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1",TIMEOUT_SW1);
timeOUT *TO[]={&timeOut_SW0,&timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[]={&timeOut_SW0};
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and States ~~~~
#define RELAY1          D2
#define RELAY2          12
#define INPUT1          9
#define INPUT2          3

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
bool blinker_state    = false;
int delayOn           = 0;
int delayOff          = 0;

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
                        timeOut_SW0.endNow();
                        sprintf(msg, "TimeOut: IRremote[Abort]");
                        iot.pub_msg(msg);
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        timeOut_SW0.restart_to();
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
                        blinker_state = !blinker_state;
                        sprintf(msg, "IRremote: [Blinker], [%s]", blinker_state ? "ON" : "OFF");
                        iot.pub_msg(msg);
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

// ~~~~~~~~~~~~~~~  LED Switching ~~~~~~~~~~~~~~~~~~~
void turnLeds(bool state, char *txt1 = "", char *txt2 = "") {
        char msg [50];
        if(digitalRead(RELAY1)!= state || boot_overide == true) {
                digitalWrite(RELAY1, state);
                sprintf(msg, "%s: Turned[%s] %s", txt1, state ? "ON" : "OFF", txt2);
                iot.pub_msg(msg);
                boot_overide = false;
        }
}
void switch_Blinker() {
        digitalWrite(RELAY1, RelayOn);
        delay(delayOn);
        digitalWrite(RELAY1, !RelayOn);
        delay(delayOff);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
                int ext_timeout;
                for (int i=0; i<NUM_SWITCHES; i++) {
                        if (badReboot == 0) {                  // PowerOn - not a quickReboot
                                if(TO[i]->updatedTimeOUT_inFlash.getValue(ext_timeout)) {  // Read Flash
                                        if (ext_timeout!=0) {  // coded TO was changed by user
                                                TO[i]->inCode_timeout_value = ext_timeout;
                                                sprintf(mqttmsg,"Switch[#%d]-Using Updated TimeOut",i);
                                                iot.pub_err(mqttmsg);
                                        }
                                        TO[i]->restart_to();
                                }
                                else{
                                        TO[i]->updatedTimeOUT_inFlash.setValue(0);
                                }
                        }
                        else {
                                TO[i]->begin(false);
                        }
                }
                if (badReboot !=0) {                               // badReboot detected
                        iot.pub_err("badReset");
                }
        }
}
void timeOutLoop(){
        char msg_t[50];
        char msg[50];

        if(iot.mqtt_detect_reset != 2) {
                for(int i=0; i<NUM_SWITCHES; i++) {
                        swState[i] = TO[i]->looper();
                        if (swState[i]!=last_swState[i]) { // change state (ON <-->OFF)
                                if (swState[i]==1) {    // swithc ON
                                        turnLeds(1,"TimeOut");

                                }
                                else{ // switch OFF
                                        turnLeds(0,"TimeOut");
                                }
                        }
                        last_swState[i] = swState[i];
                }
        }
}
void daily_timeouts(int toff_vect[2],int ton_vect[2], byte i=0){
        time_t t=now();
        if (hour(t)==toff_vect[0] && minute(t)==toff_vect[1] && second(t)<2) {
                if (digitalRead(relays[i])==RelayOn) {
                        turnLeds(0,"Clock");
                }
        }
        if (hour(t)==ton_vect[0] && minute(t)==ton_vect[1] && second(t)<2) {
                if (digitalRead(relays[i])==!RelayOn) {
                        turnLeds(1,"Clock");
                }
        }
}

void addiotnalMQTT(char incoming_msg[50]) {
        char msg[150];
        char msg2[20];

        if      (strcmp(incoming_msg, "status") == 0) {
                if(timeOut_SW0.remain()>0) {
                        timeOut_SW0.convert_epoch2clock(now()+timeOut_SW0.remain(),now(), msg, msg2);
                        sprintf(msg2,", TimeOut [%s]", msg);
                }
                else{
                        sprintf(msg2,"");
                }
                sprintf(msg, "Status: [%s] %s", digitalRead(RELAY1) ? "ON" : "OFF", msg2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], IRremote: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_IR_REMOTE, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0 ) {
                turnLeds(1, "Switch");
        }
        else if (strcmp(incoming_msg, "off") == 0 ) {
                turnLeds(0,"Switch");
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [blink(x,y), on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "remain") == 0) {
                timeOut_SW0.convert_epoch2clock(now()+timeOut_SW0.remain(),now(), msg2, msg);
                sprintf(msg, "TimeOut: Remain [%s]", msg2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "restart_to") == 0) {
                timeOut_SW0.restart_to();
                sprintf(msg, "TimeOut: [Restart]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "end_to") == 0) {
                timeOut_SW0.endNow();
                sprintf(msg, "TimeOut: [Abort]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "restore_to") == 0) {
                timeOut_SW0.restore_to();
                timeOut_SW0.restart_to();
                sprintf(msg, "TimeOut: Restore hardCoded Value [%d mins.]", TIMEOUT_SW0);
                iot.pub_msg(msg);
                iot.sendReset("Restore");
        }
        else if (strcmp(incoming_msg, "flash") == 0 ) {
                timeOut_SW0.inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                timeOut_SW0.inCodeTimeOUT_inFlash.format();
        }

        else{
                iot.inline_read(incoming_msg);
                if(strcmp(iot.inline_param[0],"timeout") == 0 && atoi(iot.inline_param[1])>0) {
                        timeOut_SW0.setNewTimeout(atoi(iot.inline_param[1]));
                        timeOut_SW0.convert_epoch2clock(now()+atoi(iot.inline_param[1]),now(), msg2, msg);
                        sprintf(msg, "TimeOut: new TimeOut Added %s", msg2);
                        iot.pub_msg(msg);
                }
                else if(strcmp(iot.inline_param[0],"blink") == 0 && atoi(iot.inline_param[1])>0 && atoi(iot.inline_param[2])>0) {
                        turnLeds(0,"Blink");
                        delayOn=atoi(iot.inline_param[1]);
                        delayOff=atoi(iot.inline_param[2]);

                        sprintf(msg, "Mode: [%s]", "Blink");
                        blinker_state = !blinker_state;
                        iot.pub_msg(msg);
                }
                else if(strcmp(iot.inline_param[0],"updateTO") == 0 && atoi(iot.inline_param[1])>0) {
                        timeOut_SW0.updateTOinflash(atoi(iot.inline_param[1]));
                        sprintf(msg, "TimeOut: Updated in flash to [%d min.]", atoi(iot.inline_param[1]));
                        iot.pub_msg(msg);
                        delay(1000);
                        iot.sendReset("TimeOut update");
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
void setup() {
        startGPIOs();
        start_IR();
        quickPwrON();
        startIOTservices();
}
void loop() {
        iot.looper();   // iot connection/Wifi/NTP/MQTT services

        recvIRinputs(); // IR signals
        timeOutLoop();

        if (checkbadReboot == true && USE_RESETKEEPER == true) {
                recoverReset();
        }
        if (USE_DAILY_TO == true) {
                daily_timeouts(CLOCK_OFF, CLOCK_ON,0);
        }
        if (USE_INPUTS == true) {
                // checkSwitch_Pressed();
        }

        delay(100);
}
