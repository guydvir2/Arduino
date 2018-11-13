//change deviceTopic !
//###################################################
#define deviceTopic "HomePi/Dvir/Windows/ota_Room"
//###################################################
#define RelayOn LOW
#define SwitchOn LOW

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <Ticker.h>

// ADDED OTA
// #include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
bool OTAflag;
unsigned int timeoutOTA = 0;

//#####################


// GPIO Pins for ESP8266
const int inputUpPin = 4;
const int inputDownPin = 5;
const int outputUpPin = 14;
const int outputDownPin = 12;
const byte progPin = 15;
//##########################


//wifi creadentials
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
//###################################


//MQTT broker parameters
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";
// ######################################


// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* deviceName = deviceTopic;
const char* topicArry[]={deviceTopic,groupTopic};
char stateTopic[50];
char availTopic[50];
// ##############################################


// GPIO status flags
bool outputUp_currentState;
bool outputDown_currentState;
bool inputUp_lastState;
bool inputDown_lastState;
bool inputUp_currentState;
bool inputDown_currentState;
// ###########################

// time interval parameters
const int clockUpdateInt=1; // hrs to update clock
int timeInt2Reset = 1500; // time between consq presses to init RESET cmd
long MQTTtimeOut = (1000*60)*5; //5 mins stop try to MQTT
long WIFItimeOut = (1000*60)*2; //2 mins try to connect WiFi
int deBounceInt = 50; //
volatile int wdtResetCounter = 0;
int wdtMaxRetries =3;
// ############################


// RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 5; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
long resetTimer = 0;
// ####################


// MQTT connection flags
int mqttFailCounter = 0; // count tries to reconnect
long firstNotConnected = 0; // time stamp of first try
int connectionFlag = 0;
int MQTTretries = 3; // allowed tries to reconnect
// ######################

// assorted
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
const char *ver="ESP_1.9";
// ###################


// %%%%%%%%%%%%%%%%%%%%%%%% Start Here ##################

//  Start Modules
WiFiClient espClient;
PubSubClient client(espClient);
Ticker wdt;

void setup() {
        Serial.begin(9600);

        startGPIOs();
        startNetwork();
        PBit(); // PowerOn Bit
        wdt.attach(1,takeTheDog); // Start WatchDog

        startOTA(); // OTA service
        // if (digitalRead(progPin)==SwitchOn) {
        OTAflag = true;
        timeoutOTA = millis();
        // }

}
void loop() {
        wdtResetCounter = 0; // reset WDT
        wait4OTA();
        verifyMQTTConnection();
        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF
        checkSwitch_PressedUp();
        checkSwitch_PressedDown();

        delay(50);
}

// GPIO
void startGPIOs(){
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        pinMode(progPin, INPUT_PULLUP);

        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
}
void PBit(){
        int pause = 5*deBounceInt;
        allOff();

        digitalWrite(outputUpPin, RelayOn);
        delay(pause);
        digitalWrite(outputUpPin, !RelayOn);
        delay(pause);
        digitalWrite(outputDownPin, RelayOn);
        delay(pause);

        allOff();

}
void allOff() {
        digitalWrite(outputUpPin,!RelayOn);
        digitalWrite(outputDownPin,!RelayOn);
        inputUp_lastState = digitalRead(inputUpPin);
        inputDown_lastState = digitalRead(inputDownPin);
}
void readGpioStates(){
        outputUp_currentState = digitalRead(outputUpPin);
        outputDown_currentState = digitalRead(outputDownPin);
        inputDown_currentState = digitalRead(inputDownPin);
        inputUp_currentState = digitalRead(inputUpPin);
}
//

// Switch GPIOs
void switchIt(char *type, char *dir){
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir,"up")==0) {
                states[0]=RelayOn;
                states[1]=!RelayOn;
        }
        else if (strcmp(dir,"down")==0) {
                states[0]=!RelayOn;
                states[1]=RelayOn;
        }
        else if (strcmp(dir,"off")==0) {
                states[0]=!RelayOn;
                states[1]=!RelayOn;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (outputUp_currentState != states[0] && outputDown_currentState != states[1]) {
                allOff();

                delay(deBounceInt*2);
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (outputUp_currentState != states[0] || outputDown_currentState != states[1]) {
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }

        client.publish(stateTopic, dir, true);
        sprintf(mqttmsg,"[%s] switched [%s]",type, dir);
        pub_msg(mqttmsg);
}
void checkSwitch_PressedUp() {
        bool temp_inputUpPin = digitalRead(inputUpPin);

        if (temp_inputUpPin != inputUp_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpPin) != inputUp_lastState) {
                        if (digitalRead(inputUpPin) == SwitchOn && outputUp_currentState == !RelayOn) {
                                switchIt("Button","up");
                                detectResetPresses();
                                lastResetPress = millis();
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                        else if (digitalRead(inputUpPin) == !SwitchOn && outputUp_currentState == RelayOn) {
                                switchIt("Button","off");
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                }

                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg,"UP Bounce detected: cRead[%s] lRead[%s]", temp_inputUpPin, inputUp_lastState);
                        pub_msg(tMsg);
                }
        }

}
void checkSwitch_PressedDown(){
        bool temp_inputDownPin = digitalRead(inputDownPin);

        if (temp_inputDownPin != inputDown_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputDownPin) != inputDown_lastState) {
                        if (digitalRead(inputDownPin) == SwitchOn && outputDown_currentState == !RelayOn) {
                                switchIt("Button","down");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                        else if (digitalRead(inputDownPin) == !SwitchOn && outputDown_currentState == RelayOn) {
                                switchIt("Button","off");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                }

                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg,"Down Bounce detected: cRead[%s] lRead[%s]", temp_inputDownPin, inputDown_lastState);
                        pub_msg(tMsg);
                }
        }
}
void verifyNotHazardState(){
        if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn ) {
                switchIt("Button","off");
                Serial.println("Hazard state - both switches were ON");
                pub_msg("HazradState - Reset");
                sendReset();
        }

}
//

//  OTA FUNCTIONS
void startOTA() {

        // Port defaults to 8266
        // ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        ArduinoOTA.setHostname("Room1");

        // No authentication by default
        ArduinoOTA.setPassword("admin");

        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

        ArduinoOTA.onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH) {
                        type = "sketch";
                } else { // U_SPIFFS
                        type = "filesystem";
                }

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
        });
        ArduinoOTA.onEnd([]() {
                Serial.println("\nEnd");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) {
                        Serial.println("Auth Failed");
                } else if (error == OTA_BEGIN_ERROR) {
                        Serial.println("Begin Failed");
                } else if (error == OTA_CONNECT_ERROR) {
                        Serial.println("Connect Failed");
                } else if (error == OTA_RECEIVE_ERROR) {
                        Serial.println("Receive Failed");
                } else if (error == OTA_END_ERROR) {
                        Serial.println("End Failed");
                }
        });
        ArduinoOTA.begin();
        Serial.println("Ready");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

}
void wait4OTA() {
        if (OTAflag == true && millis()-timeoutOTA < 90000) {
                ArduinoOTA.handle();
                digitalWrite(progPin,!digitalRead(progPin));
                delay(100);
        }
        else if (millis()-timeoutOTA > 90000) {
          OTAflag = false;
        }
}
// ##################

// Networking
void startNetwork() {
        long startWifiConnection=0;

        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);

        startWifiConnection = millis();
        WiFi.begin(ssid, password);
        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis()-startWifiConnection <WIFItimeOut) {
                delay(500);
                Serial.print(".");
        }

        WiFi.setAutoReconnect(true);
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        startMQTT();
        startNTP();

        get_timeStamp();
        strcpy(bootTime,timeStamp);
}
void startNTP() {
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000*3600*clockUpdateInt);
}
//

// MQTT
void startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
}
int connectMQTT() {
        // verify wifi connected
        if (WiFi.status() == WL_CONNECTED) {
                Serial.println("have wifi, entering MQTT connection");
                while (!client.connected() && mqttFailCounter <= MQTTretries) {
                        Serial.print("Attempting MQTT connection...");
                        // Attempt to connect
                        if (client.connect(deviceName,user, passw, availTopic,0,true,"offline")) {
                                Serial.println("connected");
                                client.publish(availTopic, "online", true);
                                if (firstRun == true) {
                                        client.publish(stateTopic, "off", true);
                                        firstRun = false;
                                }
                                pub_msg("Connected to MQTT server");
                                for (int i=0; i<sizeof(topicArry)/sizeof(char *); i++) {
                                        client.subscribe(topicArry[i]);
                                        sprintf(msg, "Subscribed to %s",topicArry[i]);
                                }
                                mqttFailCounter = 0;
                                return 1;
                        }
                        else {
                                Serial.print("failed, rc=");
                                Serial.print(client.state());
                                Serial.println(" try again in 5 seconds");
                                delay(5000);
                                Serial.print("number of fails to reconnect MQTT");
                                Serial.println(mqttFailCounter);
                                mqttFailCounter++;
                        }
                }
                Serial.println("Exit without connecting MQTT");
                mqttFailCounter = 0;
                return 0;
        }
        else {
                Serial.println("Not connected to Wifi, abort try to connect MQTT broker");
                return 0;
        }
}
void verifyMQTTConnection(){
        //  MQTT reconnection for first time or after first insuccess to reconnect
        if (!client.connected() && firstNotConnected == 0) {
                connectionFlag=connectMQTT();
//  still not connected
                if (connectionFlag == 0 ) {
                        firstNotConnected=millis();
                }
                else {
                        client.loop();
                }
        }
// retry after fail - resume only after timeout
        else if (!client.connected() && firstNotConnected !=0 && millis() - firstNotConnected > MQTTtimeOut) {
//    after cooling out period - try again
                connectionFlag=connectMQTT();
                firstNotConnected = 0;
                Serial.println("trying again to reconnect");
        }
        else {
                client.loop();
        }
}
void callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[50];
        char state[5];
        char state2[5];
        char msg2[100];

//      Display on Serial monitor only
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length]=0;
        Serial.println("");
//      ##############################

//      status - via MQTT
        if (strcmp(incoming_msg,"status")==0) {
                // relays state
                if (outputUp_currentState==RelayOn && outputDown_currentState==RelayOn) {
                        sprintf(state,"invalid Relay State");
                }
                else if (outputUp_currentState==!RelayOn && outputDown_currentState==RelayOn) {
                        sprintf(state,"DOWN");
                }
                else if (outputUp_currentState==RelayOn && outputDown_currentState==!RelayOn) {
                        sprintf(state,"UP");
                }
                else {
                        sprintf(state,"OFF");
                }

                // switch state
                if (inputUp_lastState==!RelayOn && inputDown_lastState==!RelayOn) {
                        sprintf(state2, "OFF");
                }
                else if (inputUp_lastState==RelayOn && inputDown_lastState==!RelayOn) {
                        sprintf(state2, "UP");
                }
                else if (inputUp_lastState==!RelayOn && inputDown_lastState==RelayOn) {
                        sprintf(state2, "DOWN");
                }

                // sprintf(msg,"Status: bootTime:[%s], Relay:[%s], Switch:[%s], Ver:[%s]",bootTime, state,state2,ver);
                sprintf(msg,"Status: Relay:[%s], Switch:[%s]", state,state2);
                pub_msg(msg);
        }
//      switch commands via MQTT
        else if (strcmp(incoming_msg,"up")==0 || strcmp(incoming_msg,"down")==0 || strcmp(incoming_msg,"off")==0) {
                switchIt("MQTT",incoming_msg);
        }
        else if (strcmp(incoming_msg, "info") == 0 ) {
                sprintf(msg, "info: boot:[%s,  Ver:[%s]", bootTime, ver);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg,"reset")==0 ) {
                sendReset();
        }
}
//

// Publish MQTT MSG
void pub_msg(char *inmsg){
        char tmpmsg[150];

        get_timeStamp();
        sprintf(tmpmsg,"[%s] [%s]",timeStamp,deviceTopic );
        msgSplitter(inmsg, 95, tmpmsg, "split #" );
}
void createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state,"%s/State",devTopic);
        sprintf(avail,"%s/Avail",devTopic);
}
void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg){
        char tmp[120];

        if (strlen(prefix) + strlen(msg_in) > max_msgSize) {
                int max_chunk = max_msgSize - strlen(prefix) - strlen(split_msg);
                int num=ceil((float)strlen(msg_in)/max_chunk);
                int pre_len;

                for (int k=0; k<num; k++) {
                        sprintf(tmp,"%s %s%d: ",prefix,split_msg,k);
                        pre_len = strlen(tmp);
                        for (int i=0; i<max_chunk; i++) {
                                tmp[i+pre_len]=(char)msg_in[i+k*max_chunk];
                                tmp[i+1+pre_len]='\0';
                        }
                        client.publish(msgTopic, tmp);

                }
        }
        else {
                sprintf(tmp,"%s %s",prefix,msg_in);
                client.publish(msgTopic, tmp);
        }
}
void get_timeStamp(){
        time_t t=now();
        sprintf(timeStamp,"%02d-%02d-%02d %02d:%02d:%02d",year(t),month(t), day(t), hour(t), minute(t), second(t));
}


// WatchDog
void takeTheDog(){
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset();
        }
}
void detectResetPresses(){
        if (millis()-lastResetPress < timeInt2Reset) {
                Serial.println(millis()-lastResetPress);
                if (manResetCounter >=pressAmount2Reset) {
                        sendReset();
                        manResetCounter=0;
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}
void sendReset() {
        Serial.println("Sending Reset command");
        ESP.restart();
}
//
