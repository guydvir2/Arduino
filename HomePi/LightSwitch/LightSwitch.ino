#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/test"

//~~~Services
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_MAN_RESET    false
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false

//~~~Selecto Board
#define SONOFF_DUAL      false
#define SONOFF_BASIC     true
#define WEMOS            false
// ~~~

//~~~ Select Switches
#define NUM_SWITCHES 1
int relay_timeout[] = {-1, 60}; // -1:on, 0:off <0:timeout mins
//~~~

#define VER "SonoffDual_1.4"
//####################################################


#if (SONOFF_DUAL)

// state definitions
#define RelayOn       LOW
#define SwitchOn      LOW
#define LedOn         LOW
#define ButtonPressed LOW

#define RELAY1  5
#define RELAY2  12
#define BLUEled 13

#define INPUT1  9
#define INPUT2  0
#define BUTTON  10

#endif


#if (SONOFF_BASIC)
// state definitions
#define RelayOn       LOW
#define SwitchOn      LOW
#define LedOn         LOW
#define ButtonPressed LOW

#define RELAY1  12
#define RELAY2  -1
#define BLUEled 13

#define INPUT1  14
#define INPUT2  -1
#define BUTTON  0

#endif

int relays[] = {RELAY1, RELAY2};
byte inputs[] = {INPUT1, INPUT2};
int inputs_lastState[NUM_SWITCHES];
long start_timeout[] = {0, 0}; // store clock start

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

void setup() {
        startGPIOs();
        timeout_atBoot();

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (relay_timeout[i] > 0 ) {
                        switchIt("TimeOut", i, "on");
                }
                else if (relay_timeout[i] == -1 ) {
                        switchIt("atBoot", i, "on");
                }
        }
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                if (relay_timeout[i] == 0) {
                        digitalWrite(relays[i], RelayOn);
                }
                else if ( relay_timeout[i] == -1) {
                        digitalWrite(relays[i], !RelayOn);
                }

                pinMode(inputs[i], INPUT_PULLUP);
                inputs_lastState[i] = digitalRead(inputs[i]);
        }

        if (SONOFF_DUAL) {
                pinMode(BUTTON, INPUT_PULLUP);
                pinMode(BLUEled, OUTPUT);
                digitalWrite(BLUEled,!LedOn);
        }

}

void timeout_atBoot() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (relay_timeout[i] > 0) {
                        digitalWrite(relays[i], RelayOn);
                        start_timeout[i] = millis();
                }
        }
}

void timeoutLoop() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (start_timeout[i] != 0 ) {
                        if ((millis() - start_timeout[i] ) > relay_timeout[i] * 1000 * 60 ) {
                                switchIt("TimeOut", i, "off");
                                start_timeout[i] = 0;
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
                digitalWrite(relays[i], RelayOn);
                delay(pause);
                digitalWrite(relays[i], !RelayOn);
                delay(pause);
        }

        allOff();
}

void allOff() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                digitalWrite(relays[i], !RelayOn);
                inputs_lastState[i] = digitalRead(inputs[i]);
        }
}

void switchIt(char *type, int sw_num, char *dir) {
        char mqttmsg[50];
        char states[150];
        char tempstr[150];

        if (sw_num < NUM_SWITCHES && sw_num >= 0) {
                if (strcmp(dir, "on") == 0) {
                        digitalWrite(relays[sw_num], !RelayOn);
                }
                else if (strcmp(dir, "off") == 0) {
                        digitalWrite(relays[sw_num], RelayOn);
                }

                if (iot.mqttConnected == true) {
                        sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num + 1, dir);
                        if (strcmp(type, "TimeOut") == 0) {
                                sprintf(tempstr, " [%d min.]", relay_timeout[sw_num]);
                                strcat(mqttmsg, tempstr);
                        }
                        for (int i = 0; i < NUM_SWITCHES; i++) {
                                sprintf(tempstr, "Switch#[%d] [%s] ", i + 1, !digitalRead(relays[i]) ? "On" : "Off");
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
                                        switchIt("Button", 1, "off");
                                        inputs_lastState[i] = digitalRead(inputs[i]);
                                }
                        }

                }

                else if (USE_BOUNCE_DEBUG) { // for debug only
                        char tMsg [100];
                        //      sprintf(tMsg, "1 Bounce: cRead [%d] lRead[%d]", digitalRead(inputPin_1), input1_lastState);
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

void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        timeoutLoop();

        if (iot.mqttConnected) {
                digitalWrite(BLUEled, LedOn);
        }

        if (USE_EXT_BUTTONS) {
                checkSwitch_Pressed();
        }

        if (digitalRead(BUTTON) == ButtonPressed) {
                iot.sendReset("Reset by Button");
        }

        delay(100);
}
