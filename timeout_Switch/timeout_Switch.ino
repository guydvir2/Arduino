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
// ~~~

// ~~~ define onBoot timeout in [minutes] ~~~~
/* -1 ==  on
    0 ==  off
    >0 == timeout
 */
int relay_timeout[] = {2,2}; //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define VER "SonoffBasic_2.0_alpha"
//####################################################

//~~~~~~~~~~~~~~ myJSON  ~~~~~~~~~~~~~~~~~~~~~
#define jfile "myfile.json"
#define BOOT_CALC_KEY       "BootTime1"
#define BOOT_RESET_KEY      "BootTime2"
#define TIMEOUT0_KEY         "savedTimeOut0"
#define TIMEOUT1_KEY         "savedTimeOut1"
#define SWITCH0_STATE_KEY    "savedSwitch0_State"
#define SWITCH1_STATE_KEY    "savedSwitch1_State"


// Read from FLASH
long savedBoot_Calc     = 0; // boot time for calc
long savedBoot_reset    = 0; // last actual boot
long savedTimeOut       = 0; // clock when stopwatch start
long clockShift         = 0;

int resetIntervals = 30; //secץ time between reboots to be considered as continuation
// int allowedTime_betweenReboots = 99999; // considered as inf time
// int allowedTime_betweenReboots = 0; // considered as every reboot an new operation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
long updated_bootTime        = 0;
bool resetBoot_flag       = false;

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


int relays[] = {RELAY1, RELAY2};
byte inputs[] = {INPUT1, INPUT2};
int inputs_lastState[NUM_SWITCHES];
long start_timeout[] = {0, 0}; // store clock start for TO
long end_timeout[] = {0, 0};
// manual RESET parameters
int manResetCounter =               0;  // reset press counter
int pressAmount2Reset =             3; // time to press button to init Reset
long lastResetPress =               0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
const int deBounceInt =             50; // mili
// ####################

char parameters[2][4]; //values from user

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
myJSON json(jfile, true);

void setup() {
  load_allFLASH_vars();
        startGPIOs();

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);  // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker



        // ~~~~~~~~~~~~~ using switchIt just to notify MQTT  ~~~~~~~
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (relay_timeout[i] > 0 ) {
                        switchIt("TimeOut@Boot", i, "on");
                }
                else if (relay_timeout[i] == -1 ) {
                        switchIt("on@Boot", i, "on");
                }
        }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                if (relay_timeout[i] == 0) {  // OFF STATE
                        digitalWrite(relays[i], !RelayOn);
                        if(USE_FAT) {
                                if(i==0) {
                                        json.setValue(SWITCH0_STATE_KEY,0);
                                }
                                else if(i==1) {
                                        json.setValue(SWITCH1_STATE_KEY,0);
                                }
                        }
                }
                else if ( relay_timeout[i] == -1) { // ON STATE
                        digitalWrite(relays[i], RelayOn);
                        if(USE_FAT) {
                                if(i==0) {
                                        json.setValue(SWITCH0_STATE_KEY,1);
                                }
                                else if(i==1) {
                                        json.setValue(SWITCH1_STATE_KEY,1);
                                }
                        }

                }
                else if (relay_timeout[i] > 0) { // TimeOut STATE [minuntes]
                        digitalWrite(relays[i], RelayOn);
                        start_timeout[i] = millis();
                        end_timeout [i] = updated_bootTime + relay_timeout[i]*60;
                        if(USE_FAT) {
                                if(i==0) {
                                        json.setValue(SWITCH0_STATE_KEY,1);
                                }
                                else if(i==1) {
                                        json.setValue(SWITCH1_STATE_KEY,1);
                                }
                        }
                }

                pinMode(inputs[i], INPUT_PULLUP);
                inputs_lastState[i] = digitalRead(inputs[i]);
        }

        if (SONOFF_DUAL) {
                pinMode(BUTTON, INPUT_PULLUP);
                pinMode(wifiOn_statusLED, OUTPUT);
                digitalWrite(wifiOn_statusLED, LedOn);
        }

}
void timeoutLoop() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                // if (start_timeout[i] != 0 ) {
                //         if ((millis() - start_timeout[i] ) > relay_timeout[i] * 1000 * 60 ) {
                //                 switchIt("TimeOut", i, "off");
                //                 if(USE_FAT) {
                //                         if(i==0) {
                //                                 json.setValue(TIMEOUT0_KEY,0);
                //                         }
                //                         else if(i==1) {
                //                                 json.setValue(TIMEOUT01_KEY,0);
                //                         }
                //                 }
                //
                //                 start_timeout[i] = 0;
                //         }
                // }

                if (end_timeout[i] != 0 ) {
                  Serial.println(now());
                  Serial.print(i);
                  Serial.print(" :");
                  Serial.println(end_timeout[i]);
                        if (now() > end_timeout[i] ) {
                                switchIt("TimeOut", i, "off");
                                if(USE_FAT) {
                                        if(i==0) {
                                                json.setValue(TIMEOUT0_KEY,0);
                                        }
                                        else if(i==1) {
                                                json.setValue(TIMEOUT1_KEY,0);
                                        }
                                }

                                end_timeout[i] = 0;
                        }
                }
        }
}
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
                                if(sw_num==0) {
                                        json.setValue(SWITCH0_STATE_KEY,1);
                                }
                                else if(sw_num==1) {
                                        json.setValue(SWITCH1_STATE_KEY,1);
                                }
                        }
                }
                else if (strcmp(dir, "off") == 0) {
                        digitalWrite(relays[sw_num], !RelayOn);
                        if(USE_FAT) {
                                if(sw_num==0) {
                                        json.setValue(SWITCH0_STATE_KEY,0);
                                }
                                else if(sw_num==1) {
                                        json.setValue(SWITCH1_STATE_KEY,0);
                                }
                        }
                }

                if (iot.mqttConnected == true) {
                        sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num + 1, dir);
                        if (strcmp(type, "TimeOut") == 0) {
                                sprintf(tempstr, " [%d min.]", relay_timeout[sw_num]);
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
void checkSwitch_Pressed() {
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

        else { // get user defined definitions
                splitter(incoming_msg);
                if (isDigit(*parameters[1])) { // TimeOut
                        start_timeout[atoi(parameters[0]) - 1] = millis();
                        relay_timeout[atoi(parameters[0]) - 1] = atoi(parameters[1]);
                        switchIt("TimeOut", atoi(parameters[0]) - 1, "on");
                }
                else if (strcmp(parameters[1], "on") == 0 || strcmp(parameters[1], "off") == 0) {
                        switchIt("MQTT", atoi(parameters[0]) - 1, parameters[1]);
                }
        }
}

// ~~~~~~~~~~~ Load Saved Flash ~~~~~~~~~~~
bool load_bootTime() {
        if (json.getValue(BOOT_CALC_KEY, savedBoot_Calc)) {
                // Serial.print("Boot retrieved: ");
                // Serial.println(savedBoot_Calc);
        }
        else {
                json.setValue(BOOT_CALC_KEY, 0);
                // Serial.println("Boot default saved");
        }
        if (json.getValue(BOOT_RESET_KEY, savedBoot_reset)) {
                // Serial.print("Boot retrieved: ");
                // Serial.println(savedBoot_reset);
        }
        else {
                json.setValue(BOOT_CALC_KEY, 0);
                // Serial.println("Boot default saved");
        }

        long currentBootTime = now();
        int x =0;

        while (x<5) { // verify time is updated
                if (year(currentBootTime) != 1970) { //NTP update succeeded
                        json.setValue(BOOT_RESET_KEY, currentBootTime);
                        break;
                }
                else{
                        currentBootTime = now();
                }
                x +=1;
                delay(200);
        }
        if (x==4) { // fail NTP
                return 0;
        }

        if (currentBootTime - savedBoot_reset > resetIntervals ) {
                // Serial.print("TimeDelta:");
                // Serial.println(currentBootTime - savedBoot_reset);
                json.setValue(BOOT_CALC_KEY, currentBootTime);
                // Serial.println("Time updated");
                updated_bootTime = currentBootTime;                   // take clock of current boot
                clockShift = 0;
                return 1;
        }
        else if ( currentBootTime - savedBoot_reset <= resetIntervals) {
                // Serial.println("No time update");
                updated_bootTime = savedBoot_Calc;                   // take clock of last boot
                clockShift = currentBootTime - updated_bootTime;
                resetBoot_flag = true;
                return 1;
        }
}
void load_timeoutValue() {
        // if (json.getValue(TIMEOUT_KEY, savedTimeOut)) {
        //         if (savedTimeOut > 0 ) {
        //                 start_stopwatch(updated_bootTime);
        //
        //                 // Serial.print("Stopwatch retrieved: ");
        //                 // Serial.println(savedTimeOut);
        //         }
        //         else if(resetBoot_flag == false) {
        //                 json.setValue(TIMEOUT_KEY, 0);
        //                 disp_stopWatch_flag = false;
        //         }
        // }
        // else {
        //         json.setValue(TIMEOUT_KEY, 0);
        //         // Serial.println("StopWatch default saved");
        //         disp_stopWatch_flag = false;
        // }
}
void load_allFLASH_vars(){

        if (load_bootTime()==true) {
                load_timeoutValue();
        }
        else{

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
                checkSwitch_Pressed();
        }

        if (SONOFF_DUAL && digitalRead(BUTTON) == ButtonPressed) {
                iot.sendReset("Reset by Button");
        }

        delay(100);
}
