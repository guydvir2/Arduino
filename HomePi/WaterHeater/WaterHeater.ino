#include <myIOT.h>
#include <Arduino.h>
#include <Wire.h>
// #include <TimeLib.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Water"
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true

#define USE_MAN_RESET    false
#define USE_BOUNCE_DEBUG false
#define USE_OLED         true

#define VER "Wemos_3.0"
//####################################################

// state definitions
#define buttonPressed  LOW
#define relayON        HIGH
#define ledON          LOW

const int input_1Pin  = D7;
const int output_1Pin = D5;
const int ledPin      = D6;
bool relayState;
//##########################

// manual RESET parameters
int manResetCounter               = 0;  // reset press counter
int pressAmount2Reset             = 3; // time to press button to init Reset
long lastResetPress               = 0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
// ####################


// TimeOut Constants
int maxTO                    = 180; //minutes
int timeIncrements           = 15; //minutes
int timeInc_counter          = 0; // counts number of presses to TO increments
unsigned long startTime      = 0;
unsigned long endTime        = 0;
int delayBetweenPress        = 500; // consequtive presses to reset
unsigned long pressTO_input1 = 0; // TimeOUT for next press
// ##########################

time_t t_tuple;
char timeStamp [50];
char dateStamp [50];
bool sys_stateChange;
const int deBounceInt = 50;
char msg[150];

// OLED services ~~~~~
#if (USE_OLED)
#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);
char text_lines[2][20];
#endif
// ~~~~~~~~~~~~~~~~~~~
//
// // IOT services ~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// // ~~~~~~~~~~~~~~~~~~~

void setup() {
        startGPIOs();
        startOLED();
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker
}

// ~~~~ LCD ~~~~~~~
void startOLED(){
        #if (USE_OLED)
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
        #endif
}

void centeredTxtOLED(char *text, int line, int text_size = 1) {
        #if (USE_OLED)
        int line_length [2];
        int coeff_1;         //adj width
        int coeff_2;         //adj hight

        strcpy(text_lines[line], text);
        display.setTextSize(text_size);
        display.setTextColor(WHITE);

        display.clearDisplay();
        for (int i = 0; i <= 1; i++) {
                line_length[i] = strlen(text_lines[i]);
                if (text_size == 1) {
                        coeff_1 = 21;
                        coeff_2 = 8;
                }
                else if (text_size == 2) {
                        coeff_1 = 10;
                        coeff_2 = 16;
                }
                display.setCursor(((coeff_1 - line_length[i]) *(128/coeff_1)/ 2), i * coeff_2);
                display.println(text_lines[i]);
        }
        display.display();
        #endif
}

void upadteOLED_display() {
        #if (USE_OLED)
        clockString();
        char time_on_char[20];
        char time2Off_char[20];

        if (sys_stateChange) {
                display.clearDisplay();
                sys_stateChange = false;
                if (relayState == relayON ) {
                        centeredTxtOLED("ON", 0,2);
                        delay(2000);
                }
                if (relayState == !relayON ) {
                        centeredTxtOLED("OFF", 0,2);
                        delay(2000);
                }

        }

        if (relayState == relayON ) {
                int timeON = millis() - startTime;
                sec2clock(timeON, "On:", time_on_char);
                if ( timeInc_counter == 1 ) {         // ~~~~ON, no timer ~~~~~~~
                        centeredTxtOLED(timeStamp, 0,1);
                        centeredTxtOLED(time_on_char, 1,1);
                }
                else if ( timeInc_counter > 1 ) {         /// ON + Timer
                        int timeLeft = endTime - millis();
                        sec2clock(timeLeft, "Off:", time2Off_char);
                        centeredTxtOLED(time_on_char, 0,1);
                        centeredTxtOLED(time2Off_char, 1,1);
                }
        }
        else {         // OFF state - clock only
                centeredTxtOLED(timeStamp, 0,2);
                centeredTxtOLED(dateStamp, 1,2);
        }
        #endif
}
// ~~~~~~~~~~~~~~~

// ~~~~~ Services ~~~~~~~~
void startGPIOs() {
        pinMode(input_1Pin, INPUT_PULLUP);
        pinMode(output_1Pin, OUTPUT);
        pinMode(ledPin, OUTPUT);

        allOff();
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
//         sprintf(output_text, "%s %01d:%02d", text, h, m);
}
void clockString() {
        // t_tuple=now();
        // sprintf(dateStamp, "%02d-%02d-%02d", year(t_tuple), month(t_tuple), day(t_tuple));
        // sprintf(timeStamp, "%02d:%02d:%02d", hour(t_tuple), minute(t_tuple), second(t_tuple));
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        //  this method was constructed for multiple inputs- that is why
        // 1,on is ment for switch num one
        bool states[2];
        char mqttmsg[50];

        // system states:
        sys_stateChange = true;
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
                        if ( timeInc_counter == 0 && relayState == !relayON ) { // first press turns on
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
        relayState = digitalRead(output_1Pin);
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
                                        sys_stateChange = true;
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
                                sys_stateChange = true;
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
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], MAN_RESET:[%d], OLED[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_MAN_RESET, USE_OLED);
                iot.pub_msg(msg);
        }
}
void switch_1_terminator() {
        if (relayState == relayON ) {
                if ( endTime != 0 && endTime <= millis() ) {
                        switchIt("TimeOut", "1,off");
                        timeInc_counter = 0;
                        endTime = 0;
                        startTime = 0;
                }
                if(startTime>=maxTO*60*1000) {
                        switchIt("Overide TimeOut", "1,off");
                        startTime = 0;
                }
        }
}

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        iot.looper();
        readGpioStates();

        checkSwitch_1();
        switch_1_terminator(); // For Timeout operations
        upadteOLED_display();

        delay(100);
}
