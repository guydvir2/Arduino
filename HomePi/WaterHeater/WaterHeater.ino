#include <myIOT.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/WaterBoiler"
//must be defined to use myIOT
#define ADD_MQTT_FUNC addiotnalMQTT
//~~~
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_MAN_RESET false
#define USE_BOUNCE_DEBUG false

#define VER "NodeMCU_2.0"
//####################################################

// state definitions
#define buttonPressed  LOW
#define relayON  LOW
#define ledON LOW

// GPIO Pins for ESP8266/ WEMOs
const int input_1Pin = D3;
const int output_1Pin = D6;
const int ledPin = D5;
//##########################

// GPIO status flags
bool output1_currentState;
// ###########################

// manual RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
// ####################


// TimeOut Constants
int maxTO = 180; //minutes
int timeIncrements = 1; //minutes
int timeInc_counter = 0; // counts number of presses to TO increments
unsigned long startTime = 0;
unsigned long endTime = 0;
int delayBetweenPress = 500; // consequtive presses to reset
unsigned long pressTO_input1 = 0; // TimeOUT for next press
// ##########################

time_t t;
char timeStamp2 [50];
char dateStamp2 [50];
bool changeState;
const int deBounceInt = 50;
char msg[150];

LiquidCrystal_I2C lcd(0x27, 16, 2);
myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();
        startLCD();
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker
}

// ~~~~ LCD ~~~~~~~
void printCenter(char *string, int i = 0) {
        lcd.setCursor((int)(16 - strlen(string)) / 2, i);
        lcd.print(string);
}
void printLeft(char *string, int i = 0) {
        lcd.setCursor(0, i);
        lcd.print(string);
}
void printRight(char *string, int i = 0) {
        lcd.setCursor(16 - strlen(string), i);
        lcd.print(string);
}
void update_LCD() {
        clockString();
        char time_on_char[20];
        char time2Off_char[20];

        if (changeState) {
                lcd.clear();
                changeState = false;
        }

        if (output1_currentState == relayON ) {
                int timeON = millis() - startTime;
                sec2clock(timeON, "On: ", time_on_char);
                if ( timeInc_counter == 1 ) { // ~~~~ON, no timer ~~~~~~~
                        printCenter(timeStamp2, 0);
                        printCenter(time_on_char, 1);
                }
                else if ( timeInc_counter > 1 ) { /// ON + Timer
                        int timeLeft = endTime - millis();
                        sec2clock(timeLeft, "Remain: ", time2Off_char);
                        printCenter(time_on_char, 0);
                        printCenter(time2Off_char, 1);
                }
        }
        else { // OFF state - clock only
                printCenter(timeStamp2, 0);
                printCenter(dateStamp2, 1);
        }
}
// ~~~~~~~~~~~~~~~

// ~~~~~ Services ~~~~~~~~
void startGPIOs() {
        pinMode(input_1Pin, INPUT_PULLUP);
        pinMode(output_1Pin, OUTPUT);
        pinMode(ledPin, OUTPUT);

        allOff();
}
void startLCD() {
        lcd.init(); //initialize the lcd
        lcd.backlight(); //open the backligh
        lcd.setCursor(0, 0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ maintability ~~~~~~
void PBit() {
        allOff();
        digitalWrite(output_1Pin, relayON);
        delay(10);
        allOff();

}
void allOff() {
        digitalWrite(output_1Pin, !relayON);
        digitalWrite(ledPin, !ledON);
}
void sec2clock(int sec, char* text, char* output_text) {
        int h = ((int)(sec) / (1000 * 60 * 60));
        int m = ((int)(sec) - h * 1000 * 60 * 60) / (1000 * 60);
        int s = ((int)(sec) - h * 1000 * 60 * 60 - m * 1000 * 60) / 1000;
        sprintf(output_text, "%s %01d:%02d:%02d", text, h, m, s);
}
void clockString() {
        t = now();
        sprintf(dateStamp2, "%02d-%02d-%02d", year(t), month(t), day(t));
        sprintf(timeStamp2, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        //  this method was constructed for multiple inputs- that is why
        // 1,on is ment for switch num one
        bool states[2];
        char mqttmsg[50];

        // system states:
        changeState = true;
        if (strcmp(dir, "1,on") == 0) {
                digitalWrite(output_1Pin, relayON);
                digitalWrite(ledPin, ledON);
        }
        else if (strcmp(dir, "1,off") == 0) {
                digitalWrite(output_1Pin, !relayON);
                digitalWrite(ledPin, !ledON);
        }
        iot.pub_state(dir);
        sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
        iot.pub_msg(mqttmsg);
}
void checkSwitch_1() {
        if (digitalRead(input_1Pin) == buttonPressed) {
                delay(deBounceInt);
                // CASE #1 : Button is pressed. Delay creates a delay when buttons is pressed constantly
                if (digitalRead(input_1Pin) == buttonPressed && millis() - pressTO_input1 > delayBetweenPress) {
                        // CASE of it is first press and Relay was off - switch it ON, no timer.
                        if ( timeInc_counter == 0 && output1_currentState == !relayON ) { // first press turns on
                                switchIt("Button", "1,on");
                                timeInc_counter += 1;
                                startTime = millis();
                        }
                        // CASE of already on, and insde interval of time - to add timer Qouta
                        else if (timeInc_counter < (maxTO / timeIncrements) && (millis() - pressTO_input1) < 3000 ) { // additional presses update timer countdown
                                endTime = timeInc_counter * timeIncrements * 1000 * 60 + startTime;
                                timeInc_counter += 1; // Adding time Qouta
                                sec2clock((timeInc_counter - 1) * timeIncrements * 1000 * 60, "Added Timeout: +", msg);
                                iot.pub_msg(msg);
                        }
                        // CASE of time is begger that time out-  sets it OFF
                        else if (timeInc_counter >= (maxTO / timeIncrements) || (millis() - pressTO_input1) > 3000) { // Turn OFF
                                switchIt("Button", "1,off");
                                timeInc_counter = 0;
                                endTime = 0;
                                startTime = 0;
                        }
                        pressTO_input1 = millis();
                        // Disabled for now- to avoid resets by user
                        if ( USE_MAN_RESET ) { // to create an on-demand reset after number of presses / disabled due to conflict with TO presses
                                detectResetPresses();
                                lastResetPress = millis();
                        }
                }
                // CASE #2: DEBUGS
                else {
                        if (USE_BOUNCE_DEBUG) {
                                iot.pub_msg("DEBOUNCE");
                        }
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
void readGpioStates() {
        output1_currentState = digitalRead(output_1Pin);
}
void addiotnalMQTT(char *incoming_msg) {
        int swNum;
        char cmd[8];
        char tempcmd[20];
        char msg[100];

        // switch commands via MQTT
        if (isDigit(incoming_msg[0])) {
                if (incoming_msg[1] == ',') {
                        swNum = incoming_msg[0] - 48;
                        for (int i = 2; i < strlen(incoming_msg); i++) {
                                cmd[i - 2] = incoming_msg[i];
                                cmd[i - 1] = '\0';
                        }
                        if (strcmp(cmd, "on") == 0 || strcmp(cmd, "off") == 0 ) { // on/ off command only
                                sprintf(tempcmd, "%d,%s", swNum, cmd);
                                switchIt("MQTT", tempcmd);
                                if (strlen(cmd) == 2) {
                                        timeInc_counter = 1; // ON only
                                        if (startTime == 0) {
                                                startTime = millis();
                                        }
                                }
                                else {
                                        changeState = true;
                                        timeInc_counter = 0; // OFF
                                        endTime = 0;
                                        startTime = 0;
                                }
                        }
                        else {
                                timeInc_counter = atoi(cmd) / timeIncrements; // ON + Timer
                                if (startTime == 0) {
                                        startTime = millis();
                                }
                                changeState = true;
                                sprintf(tempcmd, "%d,%s", swNum, "on");
                                switchIt("MQTT", tempcmd);
                                delay(100);

                                sec2clock(timeInc_counter * timeIncrements * 1000 * 60, "Added Timeout: +", msg);
                                iot.pub_msg(msg);
                                endTime = timeInc_counter * timeIncrements * 1000 * 60 + startTime;
                        }
                }
        }

        //status - via MQTT
        else if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: Relay:[%s], Sw:[%s]", digitalRead(output_1Pin) ? "Off" : "On", digitalRead(input_1Pin) ? "Off" : "On");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: [%d], Relay: [%d], Led: [%d]", input_1Pin, output_1Pin, ledPin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], MAN_RESET:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_MAN_RESET);
                iot.pub_msg(msg);
        }
}
void switch_1_terminator() {
        if ( endTime != 0 && endTime <= millis() ) {
                switchIt("TimeOut", "1,off");
                timeInc_counter = 0;
                endTime = 0;
                startTime = 0;
        }
}

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        readGpioStates();

        checkSwitch_1();
        switch_1_terminator(); // For Timeout operations
        update_LCD();

        delay(50);
}
