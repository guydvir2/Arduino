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

//~~~Services~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false
#define USE_FAT          true // Flash Assist using JSON and FS


// ~~~~~~~~~~~~~~Start services ~~~~~~~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);


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
                        // Serial.println("a");
                        return 1;
                }
                else if (_savedTO >0 && _savedTO <=now()) {         // saved but time passed
                        switchOFF();
                        // Serial.println("b");
                        return 0;
                }
                else if (_savedTO == 0) {
                        if (val == 0) {
                                _calc_endTO = now() + _def_val;
                        }
                        else{
                                _calc_endTO=now()+val*60;
                        }
                        switchON();
                        p1.setValue(_calc_endTO);
                        // Serial.println("c");
                        return 1;
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TimeOut_var Sw_1("SW1",1);
TimeOut_var Sw_0("SW0",3);

void setup() {
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        Sw_1.begin(2);
        Sw_0.begin();
}

void timeout_looper(){
        // for(int i=0; i<sw_amount; i++) {
        //
        // }
}



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
        Sw_1.looper();
        Sw_0.looper();
        Serial.println(Sw_1.remain());
        Serial.println(Sw_0.remain());
        delay(100);
}
