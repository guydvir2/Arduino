#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// GPIOs on SONOFF board
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
const char* mqtt_server = "192.168.2.113";
char *ver = "2.2_alpha";

// Define Pin Hardware
int buttonPin = 0;

int relayPin = 12;
int ledPin = 13;
int switchPin = 14;

// Define state flags
int retries_counter = 0;
int lastSWstate = 0;
int currentSWstate;
int currentRELstate;
int asktoup_flag = 0;
int got_up_cmd = 0;
int got_down_cmd = 0;
int doff_flag = 0;
int uoff_flag = 0;

// Define MQTT topics
// const char *dev_direction = "down";
const char *dev_direction = "up";
const char *clientID = "Sonoff1";
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
                digitalWrite(ledPin, LOW);
                delay(50);
                digitalWrite(ledPin, HIGH);
                delay(50);
        }
}

void setup() {
        Serial.begin(115200); // start serial service
        delay(10);

        //  constuct topic
        createTopic(client_temp, clientID, inTopic);

        // define sonoff gpios
        pinMode(ledPin, OUTPUT);
        pinMode(relayPin, OUTPUT);
        pinMode(switchPin, INPUT_PULLUP); // GPIO14 has a pullup resistor
        pinMode(buttonPin, INPUT_PULLUP);
        digitalWrite(ledPin, HIGH); // LED high == OFF
        digitalWrite(relayPin, LOW);

        //  Switch state - at boot
        currentSWstate = digitalRead(switchPin);
        lastSWstate = digitalRead(switchPin);

        // start wifi service
        start_wifi();
        Serial.print("Code Version: ");
        Serial.println(ver);

        // start mqtt
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
}

char *status_reply() {
        char answer[5];
        char prefix;

        if (strcmp(dev_direction,"down")==0) {
                prefix='D';
        }
        else if (strcmp(dev_direction,"up")==0) {
                prefix='U';
        }

        bool ans_rel = digitalRead(relayPin);
        bool ans_led = !digitalRead(ledPin);
        bool ans_sw = !digitalRead(switchPin);
        sprintf(answer,"%c%d%d%d",prefix,ans_rel,ans_sw,ans_led);
        return answer;
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

        if (strcmp(incoming_msg,"status")==0) {
                Serial.println(status_reply());
                pubInMQTT(status_reply());
        }
        else if (strcmp(incoming_msg,"up")==0) {
                if (strcmp(dev_direction,"up")==0) {
                        got_up_cmd = 1;
                        pubInMQTT("GOT Up CMD");
                }
        }
        else if (strcmp(incoming_msg,"down")==0) {
                if (strcmp(dev_direction,"down")==0) {
                        got_down_cmd = 1;
                        pubInMQTT("GOT DOWN CMD");
                }
        }
        else if (strcmp(incoming_msg,"off")==0) {
                off_command();
                if (strcmp(dev_direction,"up")==0) {
                        pubInMQTT("UP OFF");
                }
                else if (strcmp(dev_direction,"down")==0) {
                        pubInMQTT(" DOWN OFF");
                }
        }
        else if (strcmp(incoming_msg,"askd")==0 && (strcmp(dev_direction,"down")==0)) {
                off_command();         // send local off to down
                delay(100);
                char tmp[5];
                strcpy(tmp,status_reply());
                if (tmp[0]=='D' && tmp[1]=='0') {
                        pubInMQTT("doff");
                        delay(200);
                }
        }
        else if (strcmp(incoming_msg,"doff")==0 && (strcmp(dev_direction,"up")==0)) {
                doff_flag = 1;
        }
        else if (strcmp(incoming_msg,"asku")==0 && (strcmp(dev_direction,"up")==0)) {
                off_command(); // send local off to up device
                delay(100);
                char tmp[5];
                strcpy(tmp,status_reply());
                if (tmp[0]=='U' && tmp[1]=='0') {
                        pubInMQTT("uoff");
                        delay(200);
                }
        }
        else if (strcmp(incoming_msg,"uoff")==0 && (strcmp(dev_direction,"down")==0)) {
                uoff_flag = 1;
        }
}

int on_command() {
        if (digitalRead(relayPin)==LOW) {
                digitalWrite(relayPin, HIGH);
                digitalWrite(ledPin, LOW);
                return 1;
        }
        else {
                return 0;
        }
}

int off_command() {
        if (digitalRead(relayPin)==HIGH) {
                digitalWrite(relayPin, LOW);
                digitalWrite(ledPin, HIGH);
                return 1;
        }
        else {
                return 0;
        }
}

void is_down_off(){
        pubInMQTT("askd");
}

void is_up_off(){
        pubInMQTT("asku");
}

void subscribe_mqtt(const char* topic, int qos) {
        client.subscribe(topic, qos);
        char *msg_connected = "Connected to";
        char notify[strlen(msg_connected) + strlen(topic) + 3];
        sprintf(notify, "%s: %s", msg_connected, topic);
        pubOutMQTT(notify);
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

                        subscribe_mqtt(inTopic,2);
                        subscribe_mqtt(inTopic2,0);
                        pubOutMQTT(notify);
                }
                else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        delay(5000);
                }
        }
}

void pubOutMQTT(char* msg1) {
        char outMessage[strlen(inTopic) + strlen(msg1) + 3];
        sprintf(outMessage, "[%s] %s", inTopic, msg1);
        client.publish(outTopic, outMessage,2);
}

void pubInMQTT(char* msg1) {
        char outMessage[strlen(inTopic) + strlen(msg1) + 3];
        sprintf(outMessage, "[%s][%s] %s", inTopic2,dev_direction, msg1);
        client.publish(inTopic, outMessage,2);
}

void goUp(){
        retries_counter++;
        off_command(); // local off
        is_down_off(); // send req to down off
        if (doff_flag == 1 && currentRELstate==LOW) {
                delay(200);
                on_command(); // turn High UP relay
                char report[50];
                sprintf(report,"UP: down if off:%d",doff_flag);
                pubOutMQTT(report);
                got_up_cmd = 0;
                doff_flag = 0;
        }

}

void goDown() {
        retries_counter++;
        off_command(); // local off
        is_up_off(); // send req to down off
        if (uoff_flag == 1 && currentRELstate==LOW) {
                delay(200);
                on_command(); // Turns HIGH DOWN relay
                char report[50];
                sprintf(report,"DOWN: up if off:%d",uoff_flag);
                pubOutMQTT(report);
                got_down_cmd = 0;
                uoff_flag = 0;
        }

}

void loop() {
        //  MQTT reconnection
        if (!client.connected()) {
                reconnect();
        }
        client.loop();

        currentSWstate = digitalRead(switchPin);
        currentRELstate = digitalRead(relayPin);
        digitalWrite(ledPin, !currentRELstate);

        // ############################ Physical Switch Operation #########################
        if (lastSWstate !=currentSWstate) {
                lastSWstate =currentSWstate;
                if (currentSWstate==HIGH) {
                        off_command();
                        pubOutMQTT("Switch Off");// HIGH == OFF
                }
                else if (currentSWstate==LOW) {
                        if (strcmp(dev_direction,"up")==0) {
                                goUp();
                                pubOutMQTT("Manual Switch UP");
                        }
                        else if (strcmp(dev_direction,"down")==0) {
                                goDown();
                                pubOutMQTT("Manual Switch DOWN");

                        }
                }
        } // detect change in Switch
        // ################################################################################

        // ############################ Button Operation --- Work OK ######################
        if (digitalRead(buttonPin)==LOW) {
          digitalWrite(relayPin, !currentRELstate);
          if (!currentRELstate == 0){
            pubOutMQTT("Relay Off");
          }
          else {
            pubOutMQTT("Relay On");
          }
          }
        // ################################################################################

        // // ############################ MQTT Operation ####################################
        // //Up device go UP procedure
        // if (got_up_cmd == 1 && retries_counter <10 && (strcmp(dev_direction,"up")==0)) {
        //         goUp();
        // }
        // //Down device go DOWN procedure
        // if (got_down_cmd == 1 && retries_counter <10 && (strcmp(dev_direction,"down")==0)) {
        //         goDown();
        // }
        // if (retries_counter >=10) {
        //         retries_counter = 0;
        //         got_up_cmd = 0;
        //         doff_flag = 0;
        //         got_down_cmd = 0;
        //         uoff_flag = 0;
        // }
        // ################################################################################

        delay(200);
}
