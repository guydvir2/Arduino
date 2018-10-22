#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// GPIOs on SONOFF board
int LED_SONOFF = 13;
int REL_SONOFF = 12;
int PIN_SW = 14;
int other_state = 2;

//const char* ssid = "HomeNetwork_2.4G";
//const char* password = "guyd5161";
//const char* mqtt_server = "192.168.2.113";
char *ver = "1.2";
//char

const char* ssid = "HOTBOX-Darabi";
const char* password = "75999276";
const char* mqtt_server = "guydvir.noip.me";

int lastState = 0;
int up_cmd = 0;
int down_cmd = 0;
//int off_cmd = 0;
int down_clearance = 0;
int up_clearance = 0;
//int timeout_cmd = 0 ;

const char *clientID = "Sonoff1";
const char *dev_direction = "down"; // "UP or "DOWN"

const char *client_temp = "HomePi/Dvir/Windows/";
const char *outTopic = "HomePi/Dvir/Messages";
const char* inTopic2 = "HomePi/Dvir/Windows/All";
char inTopic[50];

WiFiClient espClient;
PubSubClient client(espClient);

void createTopic(const char *chr1, const char *chr2, char *result_char) {
        strcpy(result_char, chr1);
        strcat(result_char, chr2);
}

void start_wifi() {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }

        WiFi.setAutoReconnect(true);

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        for (int i = 0; i < 3; i++) {
                digitalWrite(LED_SONOFF, LOW);
                delay(50);
                digitalWrite(LED_SONOFF, HIGH);
                delay(50);
        }
}

void setup() {
        Serial.begin(115200); // start serial service
        delay(10);

        //  constuct topic
        createTopic(client_temp, clientID, inTopic);


        // define sonoff gpios
        pinMode(LED_SONOFF, OUTPUT);
        pinMode(REL_SONOFF, OUTPUT);
        pinMode(PIN_SW, INPUT_PULLUP); // GPIO14 has a pullup resistor
        digitalWrite(LED_SONOFF, HIGH); // LED high == OFF
        digitalWrite(REL_SONOFF, LOW);

        //  Switch state - at boot
        lastState = digitalRead(PIN_SW);

        // start wifi service
        start_wifi();
        Serial.print("Code Version: ");
        Serial.println(ver);

        // start mqtt
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);

        //  run PowerOnBit
        //  PBit();
        PowerOnBlink();
}

void PowerOnBlink() {
        bool ledState = LOW;
        for (int i = 0; i < 5; i++) {
                digitalWrite(LED_SONOFF, ledState);
                delay(200);
                ledState = !ledState;
                delay(200);

        }
}
void PBit() {
        Serial.println("PB-start");
        for (int i = 0; i < 2; i++) {
                switch_on();
                delay(500);
                switch_off();
                delay(500);
        }
        Serial.println("PB-end");
}

void remote_off() {

        if (strcmp(dev_direction, "up") == 0) {
                client.publish(inTopic, "dc");
        }

        else if (strcmp(dev_direction, "down") == 0) {
                client.publish(inTopic, "uc");
        }
}

int switch_on() {
        digitalWrite(REL_SONOFF, HIGH);
        digitalWrite(LED_SONOFF, LOW); // LED ON

        if (strcmp(dev_direction, "up") == 0) {
                Serial.println("UP Is on");
                pub_mqtt(" [Up] [On]");
        }
        else if (strcmp(dev_direction, "down") == 0) {
                Serial.println("DOWN Is on");
                pub_mqtt(" [DOWN] [On]");
        }

        reset_flags();

        delay(100);
        if (digitalRead(REL_SONOFF) == HIGH) {
                return 1;
        }
        else {
                return 0;
        }
}

int switch_off() {
        digitalWrite(REL_SONOFF, LOW);
        digitalWrite(LED_SONOFF, HIGH); // LED OFF

        if (strcmp(dev_direction, "up") == 0) {
                Serial.println("UP Is on");
                pub_mqtt(" [Up] [Off]");
        }
        else if (strcmp(dev_direction, "down") == 0) {
                Serial.println("DOWN Is on");
                pub_mqtt(" [DOWN] [Off]");
        }

        delay(100);
        if (digitalRead(REL_SONOFF) == LOW) {
                return 1;
        }
        else {
                return 0;
        }
}

void callback(char* topic, byte * payload, unsigned int length) {
        char incoming_msg[50];

        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");

        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                incoming_msg[i] = (char)payload[i];
        }
        Serial.println();

        // Up device
        //  #####################################
        if (strcmp(dev_direction, "up") == 0) {
                // getting UP msg
                if (strcmp(incoming_msg, "up") == 0) {
                        up_cmd = 1;
                        Serial.println("UPCMD=1");
                }
                else if (strcmp(incoming_msg, "off") == 0) {
                        switch_off();
                        delay(500);
                        reset_flags();
                }

                //    getting replies for two down_states
                else if (strcmp(incoming_msg, "dc0") == 0) {
                        down_clearance = 0; // down relay is not off
                        Serial.println("Down-clearance = 0");
                }
                else if (strcmp(incoming_msg, "dc1") == 0) {
                        down_clearance = 1; // Down relay is off - up will be granted.
                        Serial.println("Down-clearance = 1");
                }

                //    reply for up clearance from down device
                else if (strcmp(incoming_msg, "uc") == 0) {
                        if (switch_off() == 1) {
                                client.publish(inTopic, "uc1");
                        }
                        else {
                                client.publish(inTopic, "uc0");
                        }
                }
        }


        ////  code relvant for device - Down
        else if (strcmp(dev_direction, "down") == 0) {
                // getting DOWN msg
                if (strcmp(incoming_msg, "down") == 0) {
                        down_cmd = 1;
                        Serial.println("Got DOWN CMD");
                }
                else if (strcmp(incoming_msg, "off") == 0) {
                        switch_off();
                        Serial.println("Got OFF CMD");
                        delay(500);
                        reset_flags();
                }

                //    getting reply for two up_states
                else if (strcmp(incoming_msg, "uc0") == 0) {
                        up_clearance = 0;
                        Serial.println("up-clearance = 0");
                }
                else if (strcmp(incoming_msg, "uc1") == 0) {
                        up_clearance = 1;
                        Serial.println("up-clearance = 1");
                }

                //    reply for up clearance from up device
                else if (strcmp(incoming_msg, "dc") == 0) {
                        if (switch_off() == 1) {
                                client.publish(inTopic, "dc1");
                        }
                        else {
                                client.publish(inTopic, "dc0");
                        }
                }
        }
}

void reset_flags() {
        up_cmd = 0;
        down_cmd = 0;
        //  off_cmd = 0 ;
        up_clearance = 0;
        down_clearance = 0;
//  timeout_cmd = 0 ;
}

void subscribe_mqtt(const char* topic) {
        client.subscribe(topic);
        char *msg_connected = "Connected to";
        char notify[strlen(msg_connected) + strlen(topic) + 3];
        sprintf(notify, "%s: %s", msg_connected, topic);
        pub_mqtt(notify);
}

void reconnect() {
        char tmpname[50];
        createTopic(clientID, dev_direction, tmpname);
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                if (client.connect(tmpname)) {
                        Serial.println("connected");

                        char *msg_connected = "Connected to MQTT server";
                        char notify[strlen(msg_connected) + strlen(mqtt_server) + 3];
                        sprintf(notify, "%s: %s", msg_connected, mqtt_server);
                        pub_mqtt(notify);

                        subscribe_mqtt(inTopic);
                        subscribe_mqtt(inTopic2);
                }

                else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        delay(5000);
                }
        }
}

void pub_mqtt(char* msg1) {
        char outMessage[strlen(inTopic) + strlen(msg1) + 3];
        sprintf(outMessage, "[%s] %s", inTopic, msg1);
        client.publish(outTopic, outMessage);
}

void loop() {
        //  MQTT reconnection
        if (!client.connected()) {
                reconnect();
        }
        client.loop();

        // ############# Local Switch Commands ######

        if (digitalRead(PIN_SW) != lastState) {
                if (digitalRead(PIN_SW) != digitalRead(REL_SONOFF)) {
                        //    Turn Off
                        if (digitalRead(PIN_SW) == HIGH) {
                                switch_off();
                                if (strcmp(dev_direction, "up")) {
                                        pub_mqtt("[UP] Switch [OFF]");
                                }
                                else if (strcmp(dev_direction, "down")) {
                                        pub_mqtt("[DOWN] Switch [OFF]");
                                }
                        }

                        //    Turn On
                        else if (digitalRead(PIN_SW) == LOW) {
                                remote_off();
                                if (switch_off() == 1 ) {
                                        delay(300);
                                        if (switch_on() == 1) {
                                                if (strcmp(dev_direction, "up")) {
                                                        pub_mqtt("[UP] Switch [ON]");
                                                }
                                                else if (strcmp(dev_direction, "down")) {
                                                        pub_mqtt("[DOWN] Switch [ON]");
                                                }
                                        }
                                }
                        }
                }
                lastState = digitalRead(PIN_SW);
        }

        // ############# End Local Switch ###########

        // ############# MQTT Commands ##############
        if (up_cmd == 1) {
                if (strcmp(dev_direction, "up") == 0) {
                        remote_off();
                        //      pub_mqtt("up_remote off");
                        if (switch_off() == 1 ) {
                                //        pub_mqtt("up_switched off");
                                delay(300);
                                if (switch_on() == 1) {
                                        pub_mqtt("SWITCHED UP");
                                }
                        }
                }
        }

        else if (down_cmd == 1) {
                if (strcmp(dev_direction, "down") == 0) {
                        remote_off();
                        //      pub_mqtt("down_remote off");
                        if (switch_off() == 1) {
                                //        pub_mqtt("down switch off");
                                delay(300);
                                if (switch_on() == 1) {
                                        pub_mqtt("SWITCHED DOWN");
                                }
                        }
                }
        }
        //  ########### End MQTT Commands #############
        delay(200);
}
