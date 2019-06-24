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

//##############  User Input ##################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/sono"
#define VER "NodeMCU_v1.0"

//~~~~~~~~Services~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  false
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false
#define TIMEOUT_0        1
#define TIMEOUT_1        2

#define NUM_SWITCHES     2



// ~~~~ Logic States ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define LedOn            LOW
#define ButtonPressed    LOW

// ~~~~~~ Pins Defs ~~~~~~~~
#define RELAY1           12
#define RELAY2           5
#define INPUT1           9
#define INPUT2           0
#define wifiOn_statusLED 13
int relays[2] = {RELAY1,RELAY2};
byte inputs[] = {INPUT1, INPUT2};

class TimeOut_var {
private:
int _def_val      = 0; // default value for TO ( hard coded )
long _calc_endTO  = 0; // corrected clock ( case of restart)
long _savedTO     = 0; // clock to stop TO
bool _inTO        = false;
bool _onState     = false;

public:
TimeOut_var(char *key, int def_val)
        : p1 (key)
{
        _def_val = def_val*60;//sec
}

bool looper(){
        if (_inTO == true && _calc_endTO <=now()) { // shutting down
                switchOFF();
                return 0;
        }
        else if(_inTO == true && _calc_endTO >now()) {
                return 1; // Running / ON
        }
}
bool begin(int val=0){
        if (p1.getValue(_savedTO)) {         // able to read JSON ?
                if (_savedTO > now()) {         // saved and in time
                        _calc_endTO=_savedTO;
                        switchON();
                        return 1;
                }
                else if (_savedTO >0 && _savedTO <=now()) {         // saved but time passed
                        switchOFF();
                        return 0;
                }
                else if (_savedTO == 0) {
                        if (val == 0) {
                                _calc_endTO = now() + _def_val;
                                switchON();
                                p1.setValue(_calc_endTO);
                                return 1;
                        }
                        else if (_def_val == 0) {
                                _calc_endTO = 0;
                                return 0;
                        }
                        else{
                                _calc_endTO=now()+val*60;
                                switchON();
                                p1.setValue(_calc_endTO);
                                return 1;
                        }
                }
        }
        else{         // fail to read value, or value not initialized.
                switchOFF();
                return 0;
        }
}         // not using ext. parameter
bool getStatus(){
        return _onState;
}
int remain(){
        if (_inTO == true) {
                return _calc_endTO-now();
        }
        else {
                return 0;
        }
}
void end(){
        switchOFF();
}

private:
FVars p1;
void switchON(){
        _onState = true;
        _inTO = true;
}
void switchOFF(){
        _onState = false;
        p1.setValue(0);
        _inTO = false;
}

};

// ~~~~~~~~~~~~~~Start services ~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);

TimeOut_var Sw_1("SW1",TIMEOUT_1);
TimeOut_var Sw_0("SW0",TIMEOUT_0);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const int deBounceInt               = 50; // mili
byte inputs_lastState[NUM_SWITCHES] = {0,0};
byte TO_lastState[NUM_SWITCHES]     = {0,0};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void switchIt(char *type, int sw_num, char *dir) {
        char mqttmsg[50];
        char states[50];
        char tempstr[50];

        if (sw_num < NUM_SWITCHES && sw_num >= 0) {
                if (strcmp(dir, "on") == 0 && digitalRead(relays[sw_num])==!RelayOn) {
                        digitalWrite(relays[sw_num], RelayOn);
                }
                else if (strcmp(dir, "off") == 0 && digitalRead(relays[sw_num])==RelayOn) {
                        digitalWrite(relays[sw_num], !RelayOn);
                }

        }
        if (iot.mqttConnected && strcmp(dir, "on") == 0 || strcmp(dir, "off") == 0) {
                char time[20];
                char date[20];

                sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num, dir);

                sprintf(states,"");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "On" : "Off");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
                iot.pub_msg(mqttmsg);
        }

}
void setup() {
        // startGPIOs();

        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER
        iot.start_services(ADD_MQTT_FUNC);

        // Sw_0.begin();
        // Sw_1.begin();
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);
        // }

        //         // ~~~~~ Read flash values for TimeOut ~~~~~~~~~
        //         int temp_timeout_val[]={0,0};
        //
        //         // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //
        //         if (min_timeout[i] == 0) {    // OFF STATE
        //                 digitalWrite(relays[i], !RelayOn);
        //         }
        //         else if ( min_timeout[i] == -1) {   // ON STATE
        //                 digitalWrite(relays[i], RelayOn);
        //
        //         }
        //         else if (min_timeout[i] > 0) {   // TimeOut STATE [minuntes]
        //                 // startBootCounter(i,min_timeout[i]);
        //         }
                inputs_lastState[i] = digitalRead(inputs[i]);
        }

        // if (SONOFF_DUAL) {
        //         pinMode(BUTTON, INPUT_PULLUP);
        //         pinMode(wifiOn_statusLED, OUTPUT);
        //         digitalWrite(wifiOn_statusLED, LedOn);
        // }

}
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
                inputs_lastState[i] = digitalRead(inputs[i]);
        }
}
void checkSwitch_looper() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (digitalRead(inputs[i]) != inputs_lastState[i]) {
                        delay(deBounceInt);
                        if (digitalRead(inputs[i]) != inputs_lastState[i]) {
                                if (digitalRead(inputs[i]) == SwitchOn) {
                                        switchIt("Button", i, "on");
                                        inputs_lastState[i] = digitalRead(inputs[i]);
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
                        iot.pub_err(tMsg);
                }
        }
}
void checkTimeOut_looper(TimeOut_var &TO, byte sw_num){
        TO.looper();

        if (TO.getStatus() != TO_lastState[sw_num]) {
                if (TO.getStatus()==1) {
                        switchIt("TimeOut",sw_num, "on");
                }
                else{
                        switchIt("TimeOut",sw_num, "off");
                }
        }

        TO_lastState[sw_num]=TO.getStatus();
        Serial.println(TO.remain());

}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~ MQTT  ~~~~~~~~~~~~~~~~~~
void addiotnalMQTT(char *incoming_msg) {
        char state[25];
        char msg[200];
        char tempstr[50];
        char tempstr2[50];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                strcpy(msg, "Status: ");
                // for (int i = 0; i < NUM_SWITCHES; i++) {
                //         sprintf(tempstr, "Switch#[%d] [%s], timeOut[%s] ", i, digitalRead(relays[i]) ? "On" : "Off", inTimeOut[i] ? "Yes" : "No");
                //         strcat(msg, tempstr);
                // }
                iot.pub_msg(msg);
        }
        // else {
        //         iot.inline_read (incoming_msg);
        //         if (atoi(iot.inline_param[0])>=0 && atoi(iot.inline_param[0]) < NUM_SWITCHES) {
        //                 if (strcmp(iot.inline_param[1], "on") == 0 || strcmp(iot.inline_param[1], "off") == 0) {
        //                         switchIt("MQTT", atoi(iot.inline_param[0]), iot.inline_param[1]);
        //                 }
        //
        //                 // else if (strcmp(iot.inline_param[1], "timeout") == 0) { // define counter :1,60
        //                 //         switchIt("MQTT",atoi(iot.inline_param[0]),iot.inline_param[1],atoi(iot.inline_param[2]));
        //                 // }
        //                 // else if (strcmp(iot.inline_param[1], "TOupdate") == 0) {
        //                 //         update_flash_Timeout(atoi(iot.inline_param[0]),atoi(iot.inline_param[2]));
        //                 // }
        //         }
        // }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() {
        iot.looper(); // check wifi, mqtt, wdt

        // checkSwitch_looper();
        // checkTimeOut_looper(Sw_0,0);
        // checkTimeOut_looper(Sw_1,1);

        delay(100);
}
