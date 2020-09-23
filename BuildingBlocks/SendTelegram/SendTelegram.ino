// #include <myIOT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <myLOG.h>

flashLOG LOG;

#define BOTtoken "812406965:AAEaV-ONCIru8ePuisuMfm0ECygsm5adZHs"
#define CHAT_ID "596123373"

// ********** myIOT Class ***********
// //~~~~~ Services ~~~~~~~~~~~
// #define USE_SERIAL       false
// #define USE_WDT          true
// #define USE_OTA          true
// #define USE_RESETKEEPER  true
// #define USE_FAILNTP      true
// // ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// // ~~~~~~~ MQTT Topics ~~~~~~
// #define DEVICE_TOPIC "telegram"
// #define MQTT_PREFIX  "myHome"
// #define MQTT_GROUP   "tests"
// // ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// #define ADD_MQTT_FUNC addiotnalMQTT
// myIOT iot(DEVICE_TOPIC);
// // ***************************
//
// void addiotnalMQTT(char incoming_msg[50]) {
// }
// void startIOTservices(){
//         iot.useSerial      = USE_SERIAL;
//         iot.useWDT         = USE_WDT;
//         iot.useOTA         = USE_OTA;
//         iot.useResetKeeper = USE_RESETKEEPER;
//         iot.resetFailNTP   = USE_FAILNTP;
//         strcpy(iot.prefixTopic, MQTT_PREFIX);
//         strcpy(iot.addGroupTopic, MQTT_GROUP);
//         iot.start_services(ADD_MQTT_FUNC);
// }
class myTelegram {

private:
WiFiClientSecure client;
UniversalTelegramBot bot;

private:
char _bot[100];
char _chatID[100];
char _ssid[20];
char _password[20];

int _Bot_mtbs = 1000; //mean time between scan messages
long _Bot_lasttime;   //last time messages' scan has been done
bool _Start = false;

int ledPin = 13;
int ledStatus = 0;


void handleNewMessages(int numNewMessages){
        Serial.println("handleNewMessages");
        Serial.println(String(numNewMessages));

        for (int i=0; i<numNewMessages; i++) {
                String chat_id = String(bot.messages[i].chat_id);
                String text = bot.messages[i].text;

                String from_name = bot.messages[i].from_name;
                if (from_name == "") from_name = "Guest";

                if (text == "/ledon") {
                        digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
                        ledStatus = 1;
                        bot.sendMessage(chat_id, "Led is ON", "");
                }

                if (text == "/ledoff") {
                        ledStatus = 0;
                        digitalWrite(ledPin, LOW); // turn the LED off (LOW is the voltage level)
                        bot.sendMessage(chat_id, "Led is OFF", "");
                }

                if (text == "/status") {
                        if(ledStatus) {
                                bot.sendMessage(chat_id, "Led is ON", "");
                        } else {
                                bot.sendMessage(chat_id, "Led is OFF", "");
                        }
                }

                if (text == "/start") {
                        String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
                        welcome += "This is Flash Led Bot example.\n\n";
                        welcome += "/ledon : to switch the Led ON\n";
                        welcome += "/ledoff : to switch the Led OFF\n";
                        welcome += "/status : Returns current status of LED\n";
                        bot.sendMessage(chat_id, welcome, "Markdown");
                }
        }
}

public:
myTelegram(char* Bot, char* chatID, char* ssid = "Xiaomi_D6C8", char* password = "guyd5161")
        :  bot(Bot, client)
{
        sprintf(_bot,Bot);
        sprintf(_chatID,chatID);
        sprintf(_ssid,ssid);
        sprintf(_password,password);
}
void begin(){
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        Serial.print("Connecting Wifi: ");
        Serial.println(_ssid);
        WiFi.begin(_ssid, _password);

        while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(500);
        }

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        client.setInsecure();

}

void send_msg(char *msg){
        bot.sendMessage(_chatID, msg, "");
}

void telegram_looper(){
        if (millis() > _Bot_lasttime + _Bot_mtbs)  {
                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

                while(numNewMessages) {
                        Serial.println("got response");
                        handleNewMessages(numNewMessages);
                        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                }

                _Bot_lasttime = millis();
        }
}


};

myTelegram telegram(BOTtoken, CHAT_ID);
void setup(){
        Serial.begin(115200);
        // LOG.start();
        // startIOTservices();
        telegram.begin();
        delay(10);
        telegram.send_msg("good morning Love");

}

void loop(){
  // iot.looper();
        telegram.telegram_looper();
}











// Initialize Wifi connection to the router
// char ssid[] = "Xiaomi_D6C8";     // your network SSID (name)
// char password[] = "guyd5161"; // your network key
//
// // Initialize Telegram BOT
// // #define BOTtoken "812406965:AAEaV-ONCIru8ePuisuMfm0ECygsm5adZHs"  // your Bot Token (Get from Botfather)
// #define CHAT_ID "596123373"
// // WiFiClientSecure client;
// // UniversalTelegramBot bot(BOTtoken, client);
//
// int Bot_mtbs = 1000; //mean time between scan messages
// long Bot_lasttime;   //last time messages' scan has been done
// bool Start = false;
//
// const int ledPin = 13;
// int ledStatus = 0;
//
// void handleNewMessages(int numNewMessages) {
//         Serial.println("handleNewMessages");
//         Serial.println(String(numNewMessages));
//
//         for (int i=0; i<numNewMessages; i++) {
//                 String chat_id = String(bot.messages[i].chat_id);
//                 String text = bot.messages[i].text;
//
//                 String from_name = bot.messages[i].from_name;
//                 if (from_name == "") from_name = "Guest";
//
//                 if (text == "/ledon") {
//                         digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
//                         ledStatus = 1;
//                         bot.sendMessage(chat_id, "Led is ON", "");
//                 }
//
//                 if (text == "/ledoff") {
//                         ledStatus = 0;
//                         digitalWrite(ledPin, LOW); // turn the LED off (LOW is the voltage level)
//                         bot.sendMessage(chat_id, "Led is OFF", "");
//                 }
//
//                 if (text == "/status") {
//                         if(ledStatus) {
//                                 bot.sendMessage(chat_id, "Led is ON", "");
//                         } else {
//                                 bot.sendMessage(chat_id, "Led is OFF", "");
//                         }
//                 }
//
//                 if (text == "/start") {
//                         String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
//                         welcome += "This is Flash Led Bot example.\n\n";
//                         welcome += "/ledon : to switch the Led ON\n";
//                         welcome += "/ledoff : to switch the Led OFF\n";
//                         welcome += "/status : Returns current status of LED\n";
//                         bot.sendMessage(chat_id, welcome, "Markdown");
//                 }
//         }
// }
//
//
// void setup() {
//         Serial.begin(115200);
//         client.setInsecure();
//
//         // Set WiFi to station mode and disconnect from an AP if it was Previously
//         // connected
//         WiFi.mode(WIFI_STA);
//         WiFi.disconnect();
//         delay(100);
//
//         // attempt to connect to Wifi network:
//         Serial.print("Connecting Wifi: ");
//         Serial.println(ssid);
//         WiFi.begin(ssid, password);
//
//         while (WiFi.status() != WL_CONNECTED) {
//                 Serial.print(".");
//                 delay(500);
//         }
//
//         Serial.println("");
//         Serial.println("WiFi connected");
//         Serial.print("IP address: ");
//         Serial.println(WiFi.localIP());
//
//         pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
//         delay(10);
//         digitalWrite(ledPin, LOW); // initialize pin as off
//
// }
//
// void loop() {
//         if (millis() > Bot_lasttime + Bot_mtbs)  {
//                 int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//
//                 while(numNewMessages) {
//                         Serial.println("got response");
//                         handleNewMessages(numNewMessages);
//                         numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//                 }
//
//                 Bot_lasttime = millis();
//         }
// }
