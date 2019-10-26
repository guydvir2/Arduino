#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER              "NodeMCU_5.0"
#define USE_BOUNCE_DEBUG false
#define USE_2_EXT_INPUT  true // Only for dual input window
#define USE_NOTIFY_TELE  false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC        "SaloonExit"
#define MQTT_PREFIX         "HomePi/Dvir"
#define MQTT_GROUP          "Windows"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// state definitions
#define RelayOn  LOW
#define SwitchOn LOW

// GPIO Pins for ESP8266
//~~~~Internal Switch ~~~~~~
const int inputUpPin = 4;
const int inputDownPin = 5;
const int outputUpPin = 14;
const int outputDownPin = 12;
//~~~~External Input ~~~~~~~
const int inputUpExtPin = 0;
const int inputDownExtPin = 2;
//##########################

// GPIO status flags
//~~~~Internal Switch ~~~~~~
bool inputUp_lastState;
bool inputDown_lastState;
bool inputUp_currentState;
bool inputDown_currentState;
//~~~~External Input ~~~~~~~
bool inputUpExt_lastState;
bool inputDownExt_lastState;
// ###########################

const int deBounceInt = 50;

// ~~~~~~~ Using Telegram ~~~~~~~~~~~~~~
char *Telegram_Nick = "winExit";
int time_check_messages = 2; //sec
#if USE_NOTIFY_TELE
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
void telecmds(String in_msg, String from, String chat_id, char *snd_msg) {
        String command_set[] = {"whois_online", "status", "reset", "whoami","help", "up", "down","off"};
        byte num_commands = sizeof(command_set)/sizeof(command_set[0]);
        String comp_command[num_commands];
        char prefix[100], prefix2[100];
        char t1[50], t2[50];

        sprintf(snd_msg,""); // when not meeting any conditions, has to be empty

        from.toCharArray(t1,from.length()+1);
        in_msg.toCharArray(t2, in_msg.length()+1);

        sprintf(prefix,"/%s_", Telegram_Nick);
        sprintf(prefix2,"from user: %s\ndevice replies: %s\ncommand: %s\n~~~~~~~~~~~~~~~~~~~~\n ", t1,Telegram_Nick, t2);

        for (int i=0; i < num_commands; i++) {
                comp_command[i] = prefix;
                comp_command[i] += command_set[i];
        }

        if(in_msg=="/whois_online") {
                sprintf(snd_msg,"%s%s",prefix2, Telegram_Nick);
        }
        else if (in_msg==comp_command[1]) {
                // giveStatus(t1);
                sprintf(snd_msg,"%s%s",prefix2, t1);
        } // status
        else if (in_msg==comp_command[2]) {
                sprintf(snd_msg,"%s",prefix2);
                iot.sendReset("Telegram");
        } // reset
        else if (in_msg==comp_command[3]) {
                sprintf(snd_msg,"%s~%s~ is %s",prefix2, Telegram_Nick, DEVICE_TOPIC);
        } // whoami
        else if (in_msg==comp_command[4]) {
                char t[50];
                sprintf(snd_msg,"%sCommands Available:\n", prefix2, Telegram_Nick);
                for (int i=0; i<num_commands; i++) {
                        command_set[i].toCharArray(t,30);
                        sprintf(t1,"%s\n",t);
                        strcat(snd_msg,t1);
                }
        }
        else if (in_msg==comp_command[5]) {
                switchIt("Telegram", "up");
                sprintf(snd_msg,"%s[UP]",prefix2 );
        }//up
        else if (in_msg==comp_command[6]) {
                switchIt("Telegram", "down");
                sprintf(snd_msg,"%s[DOWN]",prefix2 );
        }//down
        else if (in_msg==comp_command[7]) {
                switchIt("Telegram", "off");
                sprintf(snd_msg,"%s[OFF]",prefix2 );
        }//off
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
        startIOTservices();
        #if USE_NOTIFY_TELE
        teleNotify.begin(telecmds);
        #endif
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        if (USE_2_EXT_INPUT) {
                pinMode(inputUpExtPin, INPUT_PULLUP);
                pinMode(inputDownExtPin, INPUT_PULLUP);
        }

        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
}
void addiotnalMQTT(char incoming_msg[50]) {
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if      (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn) {
                        sprintf(state, "invalid Relay State");
                }
                else if (digitalRead(outputUpPin) == !RelayOn && digitalRead(outputDownPin) == RelayOn) {
                        sprintf(state, "DOWN");
                }
                else if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == !RelayOn) {
                        sprintf(state, "UP");
                }
                else {
                        sprintf(state, "OFF");
                }

                // switch state
                if (inputUp_lastState == !RelayOn && inputDown_lastState == !RelayOn) {
                        sprintf(state2, "OFF");
                }
                else if (inputUp_lastState == RelayOn && inputDown_lastState == !RelayOn) {
                        sprintf(state2, "UP");
                }
                else if (inputUp_lastState == !RelayOn && inputDown_lastState == RelayOn) {
                        sprintf(state2, "DOWN");
                }
                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d],ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [up, down, off, pbit]");
                iot.pub_msg(msg);

        }
}
// ~~~~ maintability ~~~~~~
void PBit() {
        int pause = 2 * 5 * deBounceInt;
        allOff();
        delay(pause);
        digitalWrite(outputUpPin, RelayOn);
        delay(pause);
        digitalWrite(outputUpPin, !RelayOn);
        delay(pause);
        digitalWrite(outputDownPin, RelayOn);
        delay(pause);
        allOff();

}
void allOff() {
        digitalWrite(outputUpPin, !RelayOn);
        digitalWrite(outputDownPin, !RelayOn);
        inputUp_lastState = digitalRead(inputUpPin);
        inputDown_lastState = digitalRead(inputDownPin);
        if (USE_2_EXT_INPUT) {
                inputUpExt_lastState = digitalRead(inputUpExtPin);
                inputDownExt_lastState = digitalRead(inputDownExtPin);
        }
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool states[2];

        if (strcmp(dir, "up") == 0) {
                states[0] = RelayOn;
                states[1] = !RelayOn;
        }
        else if (strcmp(dir, "down") == 0) {
                states[0] = !RelayOn;
                states[1] = RelayOn;
        }
        else if (strcmp(dir, "off") == 0) {
                states[0] = !RelayOn;
                states[1] = !RelayOn;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (digitalRead(outputUpPin) != states[0] && digitalRead(outputDownPin) != states[1]) {
                allOff();
                delay(deBounceInt * 2);
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (digitalRead(outputUpPin) != states[0] || digitalRead(outputDownPin) != states[1]) {
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        if (iot.mqttConnected == true) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "%s: Switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
        }
}
void checkSwitch_looper( const int &pin, char *dir, bool &lastState, char* type="Button") {
        if (digitalRead(pin) != lastState) {
                delay(deBounceInt);
                if (digitalRead(pin) != lastState) {
                        if (digitalRead(pin) == SwitchOn) {
                                switchIt(type, dir);
                                lastState = digitalRead(pin);
                        }
                        else if (digitalRead(pin) == !SwitchOn) {
                                switchIt(type, "off");
                                lastState = digitalRead(pin);
                        }
                }
                else if (USE_BOUNCE_DEBUG) { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "[%s] Bounce: cRead[%d] lRead[%d]", dir, digitalRead(pin), lastState);
                        iot.pub_err(tMsg);
                }
        }
}
void verifyNotHazardState() {
        if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn ) {
                switchIt("Button", "off");
                iot.sendReset("HazradState");
        }

}
void readGpioStates() {
        inputDown_currentState = digitalRead(inputDownPin);
        inputUp_currentState = digitalRead(inputUpPin);
}

void loop() {
        iot.looper();
        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF

        checkSwitch_looper(inputUpPin,"up", inputUp_lastState, "inButton");
        checkSwitch_looper(inputDownPin,"down", inputDown_lastState, "inButton");
        if (USE_2_EXT_INPUT) {
                checkSwitch_looper(inputUpExtPin,"up", inputUpExt_lastState, "extButton");
                checkSwitch_looper(inputDownExtPin,"down", inputDownExt_lastState, "extButton");
        }

        #if USE_NOTIFY_TELE
        teleNotify.looper();
        #endif

        delay(100);
}
