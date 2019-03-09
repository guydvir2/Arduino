#include <myIOT.h>
#include <TimeLib.h>
#include <EEPROM.h>

#include <Arduino.h>


//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/testDevice"
//must be defined to use myIOT
#define ADD_MQTT_FUNC addiotnalMQTT
//~~~
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_MAN_RESET true
#define USE_BOUNCE_DEBUG false

#define VER "NodeMCU_0.1"
//####################################################


myIOT iot(DEVICE_TOPIC);

void setup() {
  byte value;
  char timeStamp[20];
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.start_services(ADD_MQTT_FUNC);
  value = EEPROM.read(0);
  time_t t = now();
  sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
  Serial.begin(9600);
  Serial.println("HI");
  Serial.println(value);
  EEPROM.write(0,'b');
//  Serial.println(timeStamp);
}

void addiotnalMQTT(char incoming_msg[50]) {
  //        char state[5];
  //        char state2[5];
  //        char msg[100];
  //        char msg2[100];
  //
  //        if (strcmp(incoming_msg, "status") == 0) {
  //                // relays state
  //                if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn) {
  //                        sprintf(state, "invalid Relay State");
  //                }
  //                else if (digitalRead(outputUpPin) == !RelayOn && digitalRead(outputDownPin) == RelayOn) {
  //                        sprintf(state, "DOWN");
  //                }
  //                else if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == !RelayOn) {
  //                        sprintf(state, "UP");
  //                }
  //                else {
  //                        sprintf(state, "OFF");
  //                }
  //
  //                // switch state
  //                if (inputUp_lastState == !RelayOn && inputDown_lastState == !RelayOn) {
  //                        sprintf(state2, "OFF");
  //                }
  //                else if (inputUp_lastState == RelayOn && inputDown_lastState == !RelayOn) {
  //                        sprintf(state2, "UP");
  //                }
  //                else if (inputUp_lastState == !RelayOn && inputDown_lastState == RelayOn) {
  //                        sprintf(state2, "DOWN");
  //                }
  //                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
  //                iot.pub_msg(msg);
  //        }
  //        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0) {
  //                switchIt("MQTT", incoming_msg);
  //        }


}

void loop() {
  iot.looper(); // check wifi, mqtt, wdt
}
