#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC        "WaterBoiler"
#define MQTT_PREFIX         "HomePi/Dvir"
#define MQTT_GROUP          ""
#define ADD_MQTT_FUNC       addiotnalMQTT
#define USE_SERIAL          false
#define USE_WDT             true
#define USE_OTA             true
#define USE_FAT             false // Flash Assist
#define USE_RESETKEEPER     false
#define USE_FAILNTP         true

#define VER "Wemos_3.8"
myIOT iot(DEVICE_TOPIC);
//####################################################

// state definitions
#define buttonPressed      LOW
#define relayON            LOW
#define ledON              HIGH
#define USE_BOUNCE_DEBUG   false
#define USE_OLED           true

// Pin hardware Defs
const int Button_Pin     = D7;
const int Relay_Pin      = D6;
const int buttonLED_Pin  = D8;

bool relayState;
//##########################

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
        startOLED();
        startIOTservices();
}
void startGPIOs() {
        pinMode(Button_Pin, INPUT_PULLUP);
        pinMode(Relay_Pin, OUTPUT);
        pinMode(buttonLED_Pin, OUTPUT);

        allOff();
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
void switchIt(char *type, char *dir) {
        bool states[2];
        bool suc_flag = false;
        char mqttmsg[50];

        if (strcmp(dir, "on") == 0) {  // Switch On
                if(digitalRead(Relay_Pin)!=relayON) { // was OFF
                        digitalWrite(Relay_Pin, relayON);
                        digitalWrite(buttonLED_Pin, ledON);
                        if (startTime == 0) {
                                startTime = millis();
                        }
                }
                else{ // case of switching from TO mode to normal ON
                        endTime = 0;
                }
                suc_flag = true;
        }
        else if (strcmp(dir, "off") == 0 && digitalRead(Relay_Pin)==relayON) {
                digitalWrite(Relay_Pin, !relayON);
                digitalWrite(buttonLED_Pin, !ledON);
                startTime       = 0;
                timeInc_counter = 0;
                endTime         = 0;

                suc_flag = true;
        }

        if (suc_flag) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
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
                        }
                        // CASE of time is begger that time out-  sets it OFF
                        else if (timeInc_counter >= (maxTO / timeIncrements) || (millis() - pressTO_input1) > 2500) { // Turn OFF
                                switchIt("Button", "off");
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
void readGpioStates() {
        relayState = digitalRead(Relay_Pin);
}
void addiotnalMQTT(char *incoming_msg) {
        char msg[100];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: Relay:[%s], Sw:[%s]", digitalRead(Relay_Pin) ? "Off" : "On", digitalRead(Button_Pin) ? "Off" : "On");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: [%d], Relay: [%d], Led: [%d]", Button_Pin, Relay_Pin, buttonLED_Pin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], OLED[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_OLED);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0 ) {
                switchIt("MQTT", incoming_msg);
                timeInc_counter = 1;   // ON only
        }
        else if (strcmp(incoming_msg, "off") == 0 ) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "remain") == 0 ) {
                char remTime[40];
                if (endTime == 0) {
                        sec2clock(0,"Remain Time: ",remTime);
                }
                else if (endTime > 0) {
                        sec2clock(endTime-millis(),"Remain Time: ",remTime);
                }
                iot.pub_msg(remTime);
        }
        else {
                int len = iot.inline_read (incoming_msg);
                if (len == 2 && strcmp(iot.inline_param[0], "on") == 0 && atoi(iot.inline_param[1])) {
                        timeInc_counter = atoi(iot.inline_param[1]); /// timeIncrements; // ON + Timer
                        switchIt("MQTT", "on");
                        sec2clock(timeInc_counter * 1000 * 60, "Added Timeout: +", msg);
                        iot.pub_msg(msg);
                        endTime = timeInc_counter * 1000 * 60 + startTime;
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

// ~~~~~~ Loopers ~~~~~~~~~~
void loop() {
        readGpioStates();
        Switch_1_looper();
        switch_1_TimeOUToff(); // For Timeout operations

        OLEDlooper();
        iot.looper();

        delay(100);
}
