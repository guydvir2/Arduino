#include <myIOT.h>
#include <myJSON.h>
#include <ArduinoJson.h>

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "testTelegram"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
#define GET_TELEGRAM_FUNC teleFunc
myIOT iot(DEVICE_TOPIC);
myTelegram telegram(BOT_TOKEN, CHAT_ID);

// ***************************
void teleFunc(String text, String from_name, String chat_id, char sendMsg[50]){

  if (from_name == "") from_name = "Guest";

  if (text == "/ledon") {
          strcpy(sendMsg,"Led is ON");
  }

  if (text == "/start") {
          String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
          welcome += "This is Flash Led Bot example.\n\n";
          welcome += "/ledon : to switch the Led ON\n";
          welcome += "/ledoff : to switch the Led OFF\n";
          welcome += "/status : Returns current status of LED\n";
  }


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
void addiotnalMQTT(char incoming_msg[50]) {
}

void setup() {
        startIOTservices();
        telegram.begin(GET_TELEGRAM_FUNC);
        telegram.send_msg("Testing_123");
}
void loop() {
        iot.looper();
        telegram.looper();
}
