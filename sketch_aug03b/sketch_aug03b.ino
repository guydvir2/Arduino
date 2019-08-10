#include <myIOT.h>
#include <myJSON.h>
#include <ArduinoJson.h>




// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          false
#define USE_OTA          false
#define USE_RESETKEEPER  false
#define USE_FAILNTP      false
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "testJSON"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
myJSON a("file.json",true);
// ***************************


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
void addiotnalMQTT(char incoming_msg[50]) {
//        char msg[150];
//        char msg2[20];
//        if      (strcmp(incoming_msg, "status") == 0) {
//                for(int i=0; i<NUM_SWITCHES; i++) {
//                        if(TO[i]->remain()>0) {
//                                TO[i]->convert_epoch2clock(now()+TO[i]->remain(),now(), msg, msg2);
//                                sprintf(msg2,"TimeOut[%s]", msg);
//                        }
//                        else{
//                                sprintf(msg2,"");
//                        }
//                        sprintf(msg, "Status: Switch[#%d] [%s] %s",i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
//                        iot.pub_msg(msg);
//                }
//        }
//        else if (strcmp(incoming_msg, "ver") == 0 ) {
//                sprintf(msg, "ver: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA,USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
//                iot.pub_msg(msg);
//        }
//        else if (strcmp(incoming_msg, "help") == 0) {
//                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
//                iot.pub_msg(msg);
//                sprintf(msg, "Help: Commands #2 - [remain, restart_to, timeout(x), end_to, updateTO(x), restore_to]");
//                iot.pub_msg(msg);
//                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
//                iot.pub_msg(msg);
//        }
//        else if (strcmp(incoming_msg, "flash") == 0 ) {
//                TO[0]->inCodeTimeOUT_inFlash.printFile();
//        }
//        else if (strcmp(incoming_msg, "format") == 0 ) {
//                TO[0]->inCodeTimeOUT_inFlash.format();
//        }
//        else{
//                iot.inline_read(incoming_msg);
//
//                if (strcmp(iot.inline_param[1],"on") == 0 ) {
//                        switchIt("MQTT",atoi(iot.inline_param[0]),1);
//                }
//                else if (strcmp(iot.inline_param[1], "off") == 0) {
//                        switchIt("MQTT",atoi(iot.inline_param[0]),0);
//                }
//                else if(strcmp(iot.inline_param[1],"timeout") == 0) {
//                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
//                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+atoi(iot.inline_param[2])*60,now(), msg2, msg);
//                        sprintf(msg, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]),msg2);
//                        iot.pub_msg(msg);
//                }
//                else if(strcmp(iot.inline_param[1],"updateTO") == 0) {
//                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
//                        sprintf(msg, "TimeOut: Switch[%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
//                        iot.pub_msg(msg);
//                        delay(1000);
//                        iot.notifyOffline();
//                        iot.sendReset("TimeOut update");
//                }
//                else if (strcmp(iot.inline_param[1], "remain") == 0) {
//                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now()+TO[atoi(iot.inline_param[0])]->remain(),now(), msg2, msg);
//                        sprintf(msg, "TimeOut: Switch[#%d] Remain [%s]",atoi(iot.inline_param[0]), msg2);
//                        iot.pub_msg(msg);
//                }
//                else if (strcmp(iot.inline_param[1], "restart_to") == 0) {
//                        TO[atoi(iot.inline_param[0])]->restart_to();
//                        sprintf(msg, "TimeOut: Switch[#%d] [Restart]",atoi(iot.inline_param[0]));
//                        iot.pub_msg(msg);
//                        iot.notifyOffline();
//                        iot.sendReset("TimeOut restart");
//                }
//                else if (strcmp(iot.inline_param[1], "end_to") == 0) {
//                        TO[atoi(iot.inline_param[0])]->endNow();
//                        sprintf(msg, "TimeOut: Switch[#%d] [Abort]",atoi(iot.inline_param[0]));
//                        iot.pub_msg(msg);
//                }
//                else if (strcmp(iot.inline_param[1], "restore_to") == 0) {
//                        TO[atoi(iot.inline_param[0])]->restore_to();
//                        TO[atoi(iot.inline_param[0])]->restart_to();
//                        sprintf(msg, "TimeOut: Switch[#%d], Restore hardCoded Value [%d mins.]",atoi(iot.inline_param[0]), TIMEOUT_SW0);
//                        iot.pub_msg(msg);
//                        iot.notifyOffline();
//                        iot.sendReset("Restore");
//                }
//        }
}
void setup() {
startIOTservices();
// a.format();
// char retVal[20];
// a.getValue("key1",retVal);
// iot.inline_read(retVal);

// Serial.println(iot.inline_param[0]);
// Serial.println(iot.inline_param[1]);

// a.setValue("key1","11,15");
// a.setValue("key2",12);
// a.setValue("key3",13);
a.updateArray("array_key1",156);
a.updateArray("array_key1",4545);
a.updateArray("array_key2",33);

// a.nestedArray("nested_key1",5644646);
a.printFile();
a.eraseArray("array_key1");
}

void loop() {
        iot.looper();
}
