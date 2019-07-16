/*
   Upload using Generic ESP8266
   Change default 512 kB ram to
   <<<< 1024 kB of ram !!! MUST!! >>>>
   otherwise OTA will not be loaded next time
 */

#include <myIOT.h>
#include <Arduino.h>
#include <Time.h>

//####################################################
#define DEVICE_TOPIC "sonoff_test2"
#define VER          "SonoffBasic_2.0"

//~~~Services~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_MAN_RESET    false
#define USE_BOUNCE_DEBUG false
// #define USE_EXT_BUTTONS  false
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true

//~~~Select Board~~~~~~~
#define SONOFF_DUAL      false // <----- Select one
#define SONOFF_BASIC     false  // <----- Select one
#define WEMOS            true
// ~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "Lights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ TimeOuts ~~~~~~~~~
#define TIMEOUT_SW0      60*2 // mins for SW0
#define TIMEOUT_SW1      60*2 // mins for SW0

int CLOCK_OFF[2]={8,0};
int CLOCK_ON[2] ={18,0};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//####################################################

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);

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

timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
timeOUT timeOut_SW0("SW1",TIMEOUT_SW1);

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

timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);

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

timeOUT timeOut_SW0("SW0",TIMEOUT_SW0);
timeOUT timeOut_SW1("SW1",TIMEOUT_SW1);

#endif

#if NUM_SWITCHES==1
timeOUT TO[]={timeOut_SW0};
#endif
#if NUM_SWITCHES==2
timeOUT TO[]={timeOut_SW0,timeOut_SW1};
#endif

bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;
byte swState          = 0;
byte last_swState     = 0;

int relays[]  = {RELAY1, RELAY2};
byte inputs[] = {INPUT1, INPUT2};
int inputs_lastState[NUM_SWITCHES];


// manual RESET parameters
// int manResetCounter =               0;  // reset press counter
// int pressAmount2Reset =             3; // time to press button to init Reset
// long lastResetPress =               0; // time stamp of last press
// const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
// const int deBounceInt =             50; // mili
// ####################


void setup() {
        startGPIOs();
        quickPwrON(timeOut_SW0);
        #if NUM_SWITCHES==2
        quickPwrON(timeOut_SW1);
        #endif


        iot.useSerial = USE_SERIAL;
        iot.useWDT    = USE_WDT;
        iot.useOTA    = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);
                inputs_lastState[i] = digitalRead(inputs[i]);
        }

        if (SONOFF_DUAL) {
                pinMode(BUTTON, INPUT_PULLUP);
                pinMode(wifiOn_statusLED, OUTPUT);
                digitalWrite(wifiOn_statusLED, LedOn);
        }

}


// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void allOff() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                digitalWrite(relays[i], !RelayOn);
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
                }
                else if (strcmp(dir, "off") == 0) {
                        digitalWrite(relays[sw_num], !RelayOn);
                }

                sprintf(mqttmsg, "[%s] Switch#[%d] [%s]", type, sw_num + 1, dir);
                if (strcmp(type, "TimeOut") == 0) {
                        // sprintf(tempstr, " [%d min.]", relay_timeout[sw_num]);
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
// void checkSwitch_Pressed() {
//         for (int i = 0; i < NUM_SWITCHES; i++) {
//                 if (digitalRead(inputs[i]) != inputs_lastState[i]) {
//                         delay(deBounceInt);
//                         if (digitalRead(inputs[i]) != inputs_lastState[i]) {
//                                 if (digitalRead(inputs[i]) == SwitchOn) {
//                                         switchIt("Button", i, "on");
//                                         inputs_lastState[i] = digitalRead(inputs[i]);
//                                         // if (USE_MAN_RESET && i == 1) {
//                                         //         // detectResetPresses();
//                                         //         lastResetPress = millis();
//                                         // }
//                                 }
//                                 else if (digitalRead(inputs[i]) == !SwitchOn) {
//                                         switchIt("Button", i, "off");
//                                         inputs_lastState[i] = digitalRead(inputs[i]);
//                                 }
//                         }
//
//                 }
//
//                 else if (USE_BOUNCE_DEBUG) { // for debug only
//                         char tMsg [100];
//                         sprintf(tMsg, "input [%d] Bounce: current state[%d] last state[%d]", i, digitalRead(inputs[i]), inputs_lastState[i]);
//                         iot.pub_msg(tMsg);
//                 }
//         }
// }
void addiotnalMQTT(char incoming_msg[50]) {

  char msg[150];
  char msg2[20];

  if      (strcmp(incoming_msg, "status") == 0) {
    for(int i=0; i<NUM_SWITCHES;i++){
          if(TO[i].remain()>0) {
                  TO.convert_epoch2clock(now()+TO.remain(),now(), msg, msg2);
                  sprintf(msg2,", TimeOut Switch [%d] [%s]",i, msg);
          }
          else{
                  sprintf(msg2,"");
          }
          sprintf(msg, "Status: Switch [%d] [%s] %s",i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
          iot.pub_msg(msg);
        }
  }
  else if (strcmp(incoming_msg, "ver") == 0 ) {
          sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d],  ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
          iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0) {
          sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
          iot.pub_msg(msg);
          sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to]");
          iot.pub_msg(msg);
          sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
          iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "flash") == 0 ) {
          TO[0].inCodeTimeOUT_inFlash.printFile();
  }
  else if (strcmp(incoming_msg, "format") == 0 ) {
          TO[0].inCodeTimeOUT_inFlash.format();
  }

  else{
          iot.inline_read(incoming_msg);
  //
  //         // if(strcmp(iot.inline_param[0],"timeout") == 0 && atoi(iot.inline_param[1])>0) {
  //         //         timeOut_SW0.setNewTimeout(atoi(iot.inline_param[1]));
  //         //         timeOut_SW0.convert_epoch2clock(now()+atoi(iot.inline_param[1]),now(), msg2, msg);
  //         //         sprintf(msg, "TimeOut: new TimeOut Added %s", msg2);
  //         //         iot.pub_msg(msg);
  //         // }
  //         // else if(strcmp(iot.inline_param[0],"updateTO") == 0 && atoi(iot.inline_param[1])>0) {
  //         //         timeOut_SW0.updateTOinflash(atoi(iot.inline_param[1]));
  //         //         sprintf(msg, "TimeOut: Updated in flash to [%d min.]", atoi(iot.inline_param[1]));
  //         //         iot.pub_msg(msg);
  //         //         delay(1000);
  //         //         iot.sendReset("TimeOut update");
  //         // }
  // else if (strcmp(incoming_msg, "on") == 0 ) {
  //         turnLeds(1, "Switch");
  // }
  // else if (strcmp(incoming_msg, "off") == 0 ) {
  //         turnLeds(0,"Switch");
  // }

  // else if (strcmp(incoming_msg, "remain") == 0) {
  //         timeOut_SW0.convert_epoch2clock(now()+timeOut_SW0.remain(),now(), msg2, msg);
  //         sprintf(msg, "TimeOut: Remain [%s]", msg2);
  //         iot.pub_msg(msg);
  // }
  // else if (strcmp(incoming_msg, "restart_to") == 0) {
  //         timeOut_SW0.restart_to();
  //         sprintf(msg, "TimeOut: [Restart]");
  //         iot.pub_msg(msg);
  // }
  // else if (strcmp(incoming_msg, "end_to") == 0) {
  //         timeOut_SW0.endNow();
  //         sprintf(msg, "TimeOut: [Abort]");
  //         iot.pub_msg(msg);
  // }
  // else if (strcmp(incoming_msg, "restore_to") == 0) {
  //         timeOut_SW0.restore_to();
  //         timeOut_SW0.restart_to();
  //         sprintf(msg, "TimeOut: Restore hardCoded Value [%d mins.]", TIMEOUT_SW0);
  //         iot.pub_msg(msg);
  //         iot.sendReset("Restore");
  // }
  }
}
void quickPwrON(timeOUT &TO){
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */
        int temp=0;

        TO.endTimeOUT_inFlash.getValue(temp);
        if (temp>0) {
                digitalWrite(relays[0], HIGH);
        }
        else{
                digitalWrite(relays[0], LOW);
        }
}
void recoverReset(timeOUT &TO){
        // Wait for indication if it was false reset(1) or
        if(iot.mqtt_detect_reset != 2) {
                badReboot = iot.mqtt_detect_reset;
                checkbadReboot = false;

                /*
                   badReboot refers as a very short power loss / restart.
                   mostly happens due to power adaptpor momentary failure.
                   if "State" topic goes to offline - it means that powerloss was more than 10 sec
                   which equivalent to a fresh restart.
                   if "State" topic still "Online"- meaning it is a badreboot.
                 */

                int ext_timeout;
                if (badReboot == 0) { // PowerOn
                        /*
                           Not a badReboot:
                           Case a: after a fresh restart it starts over timeout default value.
                           Case b: if timeout has ended, not retstarts a new timeout, and if timeout
                           still not ended, contunue from what is left.
                         */
                        if (TO.updatedTimeOUT_inFlash.getValue(ext_timeout)) {
                                if (ext_timeout!=0) {
                                        TO.inCode_timeout_value = ext_timeout;
                                }
                                TO.restart_to();
                        }
                        else{
                                TO.updatedTimeOUT_inFlash.setValue(0);
                        }

                }
                else {
                        TO.begin(false); // badreboot detected - don't restart TO if already ended
                }
        }
}
void timeOutLoop(timeOUT &TO, int i){
        char msg_t[]="                ";
        char msg[10];

        char *states[2]={"off","on"};

        if(iot.mqtt_detect_reset != 2) {
                swState = TO.looper();
                if (swState!=last_swState) {
                        if (swState==1) { // swithc ON
                                TO.convert_epoch2clock(now()+timeOut_SW0.remain(),now(), msg_t, msg);
                                sprintf(msg,"TimeOut [%s]",msg_t);
                                switchIt(msg, i, states[i]);
                        }
                        else{ // switch OFF
                                switchIt("TimeOut Ended",i,states[i]);
                        }
                }
                last_swState = swState;
        }
}

void loop() {
        iot.looper(); // check wifi, mqtt, wdt

        if (checkbadReboot == true) {
                recoverReset(timeOut_SW0);
                #if NUM_SWITCHES==2
                recoverReset(timeOut_SW1);
                #endif
        }

        timeOutLoop(timeOut_SW0,0);
        #if NUM_SWITCHES==2
        timeOutLoop(timeOut_SW1,1);
        #endif

        if (iot.mqttConnected) {
                digitalWrite(wifiOn_statusLED, !LedOn);
        }

        // if (USE_EXT_BUTTONS) {
        //         checkSwitch_Pressed();
        // }

        if (SONOFF_DUAL && digitalRead(BUTTON) == ButtonPressed) {
                iot.sendReset("Reset by Button");
        }

        delay(100);
}
