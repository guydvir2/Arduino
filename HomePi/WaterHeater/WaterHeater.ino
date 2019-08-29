#include <myIOT.h>
#include <myJSON.h>
#include <TimeLib.h>
#include <Arduino.h>


// ********** Sketch Services  ***********
#define VER              "Wemos_4.0"
#define USE_INPUTS         false
#define USE_BOUNCE_DEBUG   false
#define USE_OLED           true
#define STATE_AT_BOOT      false

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      1*60 // mins for SW0

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "WaterBoiler2"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~~~~ TimeOuts class ~~~~~~~~~
timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
timeOUT *TO[]={&timeOut_SW0};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~~~~~~~
#if USE_OLED

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET    LED_BUILTIN

const int SCLpin       = D1;
const int SDApin       = D2;
long swapLines_counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define buttonPressed      LOW
#define relayON            LOW
#define ledON              HIGH

bool relayState;
bool swState [NUM_SWITCHES];
bool last_swState [NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];


// ~~~Pin hardware Defs ~~~~~~
const int Button_Pin       = D7;
const int Relay_Pin        = D6;
const int buttonLED_Pin    = D8;
byte relays[NUM_SWITCHES]  = {Relay_Pin};
byte inputs[NUM_SWITCHES]  = {Button_Pin};


// ~~~~ ResetKeeper Vars ~~~~~~~
bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;


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



void setup() {
        startGPIOs();
        quickPwrON();
        startOLED();
        startIOTservices();
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
        pinMode(buttonLED_Pin, OUTPUT);
}
void quickPwrON(){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */
        int temp=0;
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
                                if (STATE_AT_BOOT == true ) { // define to be ON at boot
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
                                if (swState[i]==1) { // swithc ON
                                        switchIt("TimeOut",i,1);
                                }
                                else{ // switch OFF
                                        switchIt("TimeOut",i,0);
                                }
                        }
                        last_swState[i] = swState[i];
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
                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d]: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [blink(x,y), on, off, flash, format]");
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
                        switchIt("TimeOut","on");
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("TimeOut","off");
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
                        sprintf(msg, "TimeOut: Switch[#%d], Restore hardCoded Value [%d mins.]",atoi(iot.inline_param[0]), TO[0]);
                        iot.pub_msg(msg);
                        iot.sendReset("Restore");
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

// ~~~~ OLED ~~~~~~~
void startOLED(){
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
void OLED_titleTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "") {
  #if (USE_OLED)
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        byte line_space = pow(2, (2 + char_size));


        if (strcmp(line3,"")==0 && strcmp(line4,"")==0) { // for ON state only - 2rows
                for (int n = 0; n < 2; n++) {
                        if (strcmp(Lines[n], "") != 0) {
                                if (n==1) {  // Clock line
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))),  line_space * (n +1) -3);
                                        display.print(Lines[n]);
                                }
                                else{ // Title line
                                        display.setTextSize(char_size-1);
                                        display.setTextColor(BLACK,WHITE);
                                        display.setCursor(0, line_space * (n+1));
                                        display.print(Lines[n]);
                                }
                        }
                }

        }
        else{
                for (int n = 0; n < 4; n++) {
                        if (strcmp(Lines[n], "") != 0) {
                                if (n==1 || n == 3) { // Clocks
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))),  line_space * n -3);
                                        display.print(Lines[n]);
                                }
                                else{ // Title
                                        display.setTextSize(char_size-1);
                                        display.setTextColor(BLACK,WHITE);
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

        if (relayState == relayON ) {
                int timeON = millis() - startTime;
                sec2clock(timeON, "", time_on_char);
                if ( timeInc_counter == 1 ) {         // ~~~~ON, no timer ~~~~~~~
                        OLED_titleTXT(2,"On:",time_on_char);
                }
                else if ( timeInc_counter > 1 ) {         /// ON + Timer
                        int timeLeft = endTime - millis();
                        sec2clock(timeLeft, "", time2Off_char);
                        OLED_titleTXT(2,"On:",time_on_char,"Remain:",time2Off_char);
                }
        }
        else {         // OFF state - clock only
                iot.return_clock(timeStamp);
                iot.return_date(dateStamp);

                int timeQoute=5000; // Seconds to move text on OLED
                if (swapLines_counter == 0) {
                        swapLines_counter=millis();
                }
                if (millis()-swapLines_counter < timeQoute) {
                        OLED_CenterTXT(2,"",timeStamp, dateStamp);
                }
                else if (millis()-swapLines_counter >= timeQoute &&
                         millis()-swapLines_counter <2*timeQoute) {
                        OLED_CenterTXT(2,timeStamp,"","", dateStamp);
                }
                else if (millis()-swapLines_counter > 2*timeQoute) {
                        swapLines_counter=0;
                }
        }

        #endif
}
// ~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~ string manipulations  ~~~~~~~~~~~~~~~~~~~~~~~~
void sec2clock(int sec, char* text, char* output_text) {
        int h = ((int)(sec) / (1000 * 60 * 60));
        int m = ((int)(sec) - h * 1000 * 60 * 60) / (1000 * 60);
        int s = ((int)(sec) - h * 1000 * 60 * 60 - m * 1000 * 60) / 1000;
        sprintf(output_text, "%s %01d:%02d:%02d", text, h, m, s);
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void allOff() {
        digitalWrite(Relay_Pin, !relayON);
        digitalWrite(buttonLED_Pin, !ledON);
}

void switchIt(char *txt1, int sw_num, bool state, char *txt2=""){
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word={"Turned"};

        if(digitalRead(relays[sw_num])!= state || boot_overide == true) {
                digitalWrite(relays[sw_num], state);
                digitalWrite(buttonLED_Pin, state);

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

                sprintf(states, "%s", !digitalRead(relays[i]) ? "ON" : "OFF");
                iot.pub_msg(msg);
                iot.pub_state(states);

                if (state == 1) {
                        if (startTime == 0) {
                                startTime = millis();
                        }
                }
                else {
                        startTime       = 0;
                        timeInc_counter = 0;
                        endTime         = 0;
                }
        }
}
void checkSwitch_Pressed_NEW (byte sw){
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


void Switch_1_looper() {
        if (digitalRead(Button_Pin) == buttonPressed) {
                delay(deBounceInt);
                // CASE #1 : Button is pressed. Delay creates a delay when buttons is pressed constantly
                if (digitalRead(Button_Pin) == buttonPressed && millis() - pressTO_input1 > delayBetweenPress) {
                        // CASE of it is first press and Relay was off - switch it ON, no timer.
                        if ( timeInc_counter == 0 && relayState == !relayON ) { // first press turns on
                                switchIt("Button", "on");
                                timeInc_counter += 1;
                        }
                        // CASE of already on, and insde interval of time - to add timer Qouta
                        else if (timeInc_counter < (maxTO / timeIncrements) && (millis() - pressTO_input1) < 2500 ) { // additional presses update timer countdown
                                endTime = timeInc_counter * timeIncrements * 1000 * 60 + startTime;
                                timeInc_counter += 1; // Adding time Qouta
                                sec2clock((timeInc_counter - 1) * timeIncrements * 1000 * 60, "Added Timeout: +", msg);
                                iot.pub_msg(msg);
                                TO[0]->endTimeOUT_inFlash.setValue((timeInc_counter-1) * timeIncrements);
                        }
                        // CASE of time is begger that time out-  sets it OFF
                        else if (timeInc_counter >= (maxTO / timeIncrements) || (millis() - pressTO_input1) > 2500) { // Turn OFF
                                switchIt("Button", "off");
                                TO[0]->endTimeOUT_inFlash.setValue(0);
                        }
                        pressTO_input1 = millis();
                }
                // CASE #2: DEBUGS
                else {
                        if (USE_BOUNCE_DEBUG) {
                                iot.pub_msg("DEBOUNCE");
                        }
                }
        }
}
void switch_1_TimeOUToff() {
        if (relayState == relayON ) {
                if ( endTime != 0 && endTime <= millis() ) {
                        switchIt("TimeOut", "off");
                }
                if((millis()-startTime) >= maxTO*60*1000) { // Max time to ON
                        switchIt("Overide TimeOut", "off");
                }
        }
}
void readGpioStates() {
        relayState = digitalRead(Relay_Pin);
}

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        readGpioStates();
        // Switch_1_looper();
        // switch_1_TimeOUToff(); // For Timeout operations

        OLEDlooper();
        iot.looper();
        timeOutLoop();

        if (checkbadReboot == true && USE_RESETKEEPER == true) {
                recoverReset();
        }
        if (USE_INPUTS == true) {
                checkSwitch_Pressed(0);
        }

        delay(100);
}







// void switchIt_OLD(char *type, char *dir) {
//         bool states[2];
//         bool suc_flag = false;
//         char mqttmsg[50];
//
//         if (strcmp(dir, "on") == 0) {  // Switch On
//                 if(digitalRead(Relay_Pin)!=relayON) { // was OFF
//                         digitalWrite(Relay_Pin, relayON);
//                         digitalWrite(buttonLED_Pin, ledON);
//                         if (startTime == 0) {
//                                 startTime = millis();
//                         }
//                 }
//                 else{ // case of switching from TO mode to normal ON
//                         endTime = 0;
//                 }
//                 suc_flag = true;
//         }
//         else if (strcmp(dir, "off") == 0 && digitalRead(Relay_Pin)==relayON) {
//                 digitalWrite(Relay_Pin, !relayON);
//                 digitalWrite(buttonLED_Pin, !ledON);
//                 startTime       = 0;
//                 timeInc_counter = 0;
//                 endTime         = 0;
//
//                 suc_flag = true;
//         }
//
//         if (suc_flag) {
//                 iot.pub_state(dir);
//                 sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
//                 iot.pub_msg(mqttmsg);
//         }
// }

// void addiotnalMQTT(char *incoming_msg) {
//         char msg[100];
//
//         if (strcmp(incoming_msg, "status") == 0) {
//                 sprintf(msg, "Status: Relay:[%s], Sw:[%s]", digitalRead(Relay_Pin) ? "Off" : "On", digitalRead(Button_Pin) ? "Off" : "On");
//                 iot.pub_msg(msg);
//         }
//         else if (strcmp(incoming_msg, "ver") == 0 ) {
//                 sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], OLED[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_OLED);
//                 iot.pub_msg(msg);
//         }
//         else if (strcmp(incoming_msg, "on") == 0 ) {
//                 switchIt("MQTT", incoming_msg);
//                 timeInc_counter = 1;   // ON only
//         }
//         else if (strcmp(incoming_msg, "off") == 0 ) {
//                 switchIt("MQTT", incoming_msg);
//         }
//         else if (strcmp(incoming_msg, "remain") == 0 ) {
//                 char remTime[40];
//                 if (endTime == 0) {
//                         sec2clock(0,"Remain Time: ",remTime);
//                 }
//                 else if (endTime > 0) {
//                         sec2clock(endTime-millis(),"Remain Time: ",remTime);
//                 }
//                 iot.pub_msg(remTime);
//         }
//         else {
//                 int len = iot.inline_read (incoming_msg);
//                 if (len == 2 && strcmp(iot.inline_param[0], "on") == 0 && atoi(iot.inline_param[1])) {
//                         timeInc_counter = atoi(iot.inline_param[1]); /// timeIncrements; // ON + Timer
//                         switchIt("MQTT", "on");
//                         sec2clock(timeInc_counter * 1000 * 60, "Added Timeout: +", msg);
//                         iot.pub_msg(msg);
//                         endTime = timeInc_counter * 1000 * 60 + startTime;
//                 }
//         }
// }
