#include <myIOT.h>
#include <Arduino.h>
#include <TimeLib.h>


//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/KitchenLED"

//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_IR_REMOTE    true
#define USE_TIMEOUT      false
#define USE_RESETKEEPER  true
#define TIMEOUT_1        1 // mins for SW1

#define IR_SENSOR_PIN D5
#define RelayPin      D2
#define TimeOut_ON    12 // Hours
#define LEDsON        true

#define VER "Wemos.Mini.2.0"
//####################################################

bool blinker_state      = false;
bool strobe_state       = false;
bool badReboot          = false;
bool checkbadReboot     = true;
int swState             = 0;


#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
timeOUT Sw_1("SW1",TIMEOUT_1);

// ~~~~~~~~~~~~~~~  LED Switching ~~~~~~~~~~~~~~~~~~~
void turnLeds(bool state, char * additionalTXT = "") {
        char msg [50];

        digitalWrite(RelayPin, state);
        sprintf(msg, "[%s]: Turned[%s]", additionalTXT, state? "ON":"OFF");
        iot.pub_msg(msg);
}
void switch_Blinker() {
        digitalWrite(RelayPin, LEDsON);
        delay(500);
        digitalWrite(RelayPin, !LEDsON);
        delay(50);
}
void switch_Strobes() {
        digitalWrite(RelayPin, LEDsON);
        delay(50);
        digitalWrite(RelayPin, !LEDsON);
        delay(50);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~IR Remote ~~~~~~~~~~~~~~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t kRecvPin        = IR_SENSOR_PIN;
const uint32_t kBaudRate       = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value        = 0;

IRrecv irrecv(kRecvPin);
decode_results results;

void recvIRinputs() {
#if USE_IR_REMOTE
        if (irrecv.decode(&results)) {

                if (results.value == 0XFFFFFFFF)
                        results.value = key_value;
                char msg[50];

                switch (results.value) {
                case 0xFFA25D:
                        //        Serial.println("CH-");
                        break;
                case 0xFF629D:
                        //Serial.println("CH");
                        iot.sendReset("RemoteControl");
                        break;
                case 0xFFE21D:
                        //Serial.println("CH+");
                        break;
                case 0xFF22DD:
                        //Serial.println("|<<");
                        break;
                case 0xFF02FD:
                        //Serial.println(">>|");
                        break;
                case 0xFFC23D:
                        //Serial.println(">|");
                        break;
                case 0xFFE01F:
                        //Serial.println("-");
                        turnLeds(0, "RemoteControl");
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        turnLeds(1, "RemoteControl");
                        break;
                case 0xFF906F:
                        //Serial.println("EQ");
                        break;
                case 0xFF6897:
                        //Serial.println("0");
                        break;
                case 0xFF9867:
                        //Serial.println("100+");
                        break;
                case 0xFFB04F:
                        //Serial.println("200+");
                        break;
                case 0xFF30CF:
                        blinker_state = !blinker_state;
                        sprintf(msg, "Switch: [Blinker], [%s]", blinker_state ? "ON" : "OFF");
                        iot.pub_msg(msg);
                        break;
                case 0xFF18E7:
                        strobe_state = !strobe_state;
                        sprintf(msg, "Switch: [Strobe], [%s]", strobe_state ? "ON" : "OFF");
                        iot.pub_msg(msg);
                        break;
                case 0xFF7A85:
                        break;
                case 0xFF10EF:
                        //change_color(4);
                        break;
                case 0xFF38C7:
                        //change_color(5);
                        break;
                case 0xFF5AA5:
                        //Serial.println("6");
                        //                        change_color(6);
                        break;
                case 0xFF42BD:
                        //Serial.println("7");
                        //                        change_color(7);
                        break;
                case 0xFF4AB5:
                        //Serial.println("8");
                        //                        change_color(8);
                        break;
                case 0xFF52AD:
                        //Serial.println("9");
                        //                        change_color(9);
                        break;
                }
                key_value = results.value;
                irrecv.resume();
        }
#endif
}
void start_IR() {
#if USE_IR_REMOTE
#if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
        irrecv.enableIRIn(); // Start the receiver
#endif
}

#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char incoming_msg[50]) {
        char msg[100];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: [%s]", digitalRead(RelayPin) ? "ON" : "OFF");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], IRremote: [%d], ResetKeeper[%d]", VER, iot.ver, USE_WDT, USE_OTA,
                        USE_SERIAL, USE_IR_REMOTE, USE_RESETKEEPER);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0 ) {
                turnLeds(1, "MQTT");
        }
        else if (strcmp(incoming_msg, "off") == 0 ) {
                turnLeds(0,"MQTT");
        }
        else if (strcmp(incoming_msg, "strobe") == 0 ) {
                sprintf(msg, "Mode: [%s]", "Strobe");
                strobe_state = !strobe_state;
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "blink") == 0 ) {
                sprintf(msg, "Mode: [%s]", "Blink");
                blinker_state = !blinker_state;
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, strobe, blink, on, off, ver, help, remain, end_to] , [boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "remain") == 0) {
                sprintf(msg, "Remain: [%d mins]", Sw_1.remain());
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "end_to") == 0) {
                Sw_1.end_to();
                sprintf(msg, "TimeOut: [forced END]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "end_to") == 0) {
                Sw_1.end_to();
                sprintf(msg, "TimeOut: [forced END]");
                iot.pub_msg(msg);
        }
        else{
                int a=iot.inline_read(incoming_msg);
                for(int i=0; i<a; i++) {
                        Serial.println(iot.inline_param[i]);
                }
                Sw_1.begin(1);
        }

}
void startTimeOut(){
        if (badReboot==false) { // PowerOn
                Sw_1.begin();
        }
        else {
                Sw_1.begin(0,false);
        }
}
void checkIf_badReboot(){
        // Wait for indication if it was false reset(1) or powerOn(0)
        if(iot.encounterReset !=2 && checkbadReboot) {
                badReboot = iot.encounterReset;
                checkbadReboot = false;
                startTimeOut();
        }
}

void setup() {
        pinMode(RelayPin, OUTPUT);
        digitalWrite(RelayPin, !LEDsON);

        // ~~~~~~~~Start IOT sevices~~~~~~~~
        iot.useSerial = USE_SERIAL;
        iot.useWDT    = USE_WDT;
        iot.useOTA    = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.start_services(ADD_MQTT_FUNC);

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if USE_IR_REMOTE
        start_IR();
#endif

}
void loop() {
        iot.looper();   // iot connection/Wifi/NTP/MQTT services
        checkIf_badReboot();
swState = Sw_1.looper();
        if (swState<2) {

                Serial.print("Command to switch:");
                Serial.println(swState);
                turnLeds(swState, "TimeOut");
        }
        recvIRinputs(); // IR signals

        if (strobe_state == true ) {
                switch_Strobes();
        }
        if (blinker_state == true ) {
                switch_Blinker();
        }

        delay(100);
}
