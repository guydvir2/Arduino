#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// GPIOs on SONOFF board
int LED_SONOFF = 13;
int REL_SONOFF = 12;
int PIN_SW = 14;
int other_state = 2;

const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
const char* mqtt_server = "192.168.2.113";
int last_sw_state = 0;
int up_cmd = 0;
int down_clearance = 0;
int timeout_cmd = 0 ;

const char *clientID = "Sonoff1";
const char *dev_direction = "up";

const char *client_temp = "HomePi/Dvir/Windows/";
char inTopic[50];
const char *outTopic = "HomePi/Dvir/Messages";
const char* inTopic2 = "HomePi/Dvir/Windows/All";

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
  createTopic(client_temp, clientID , inTopic);

  // define sonoff gpios
  pinMode(LED_SONOFF, OUTPUT);
  pinMode(REL_SONOFF, OUTPUT);
  pinMode(PIN_SW, INPUT_PULLUP); // GPIO14 has a pullup resistor
  digitalWrite(LED_SONOFF, HIGH); // LED high is OFF
  digitalWrite(REL_SONOFF, LOW);

  // start wifi service
  start_wifi();

  // start mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //  run PowerOnBit
  PBit();
}

void PBit() {
  for (int i=0; i<5;i++){
    switch_on();
    delay(500);
    switch_off();
    delay(500);  
  } 
}

void switch_on() {
  digitalWrite(REL_SONOFF, HIGH);
  digitalWrite(LED_SONOFF, LOW); // LED ON
  if (strcmp(dev_direction,"up")==0){
    Serial.println("UP Is on");
    pub_mqtt(" [Up] [On]");
    }
    else if (strcmp(dev_direction,"down")==0){
    Serial.println("Down Is on");
    pub_mqtt(" [Down] [On]");
    }
  
}

void switch_off() {
  digitalWrite(REL_SONOFF, LOW);
  digitalWrite(LED_SONOFF, HIGH); // LED OFF
  Serial.println("UP Is off");
  pub_mqtt("code [Up] [Off]");
}

void callback(char* topic, byte* payload, unsigned int length) {
  char incoming_msg[50];

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    incoming_msg[i] = (char)payload[i];
  }
  Serial.println();
  
  // getting UP msg
  if (strcmp(incoming_msg, "up") == 0) {
    switch_off();
    up_cmd = 1;
    client.publish(inTopic, "dc"); // asks other side for down_clearance
    Serial.println("Got UP CMD");
  }
  else if (strcmp(incoming_msg, "down") == 0) {
    switch_off();
    up_cmd = 0;
    Serial.println("Got DOWN CMD");
  }
  else if (strcmp(incoming_msg, "off") == 0) {
    switch_off();
    Serial.println("Got OFF CMD");
  }

  //    getting reply for two down_states
  else if (strcmp(incoming_msg, "dc0") == 0) {
    down_clearance = 0 ;
    Serial.println("Down-clearance = 0");
  }
  else if (strcmp(incoming_msg, "dc1") == 0) {
    down_clearance = 1 ;
    Serial.println("Down-clearance = 1");
  }

  //    reply for up clearance from down device
  else if (strcmp(incoming_msg, "up") == 0) {
    if (digitalRead(REL_SONOFF) == LOW) {
      client.publish(inTopic, "up1"); // Relay is low- you have clearance
    }
    else {
      client.publish(inTopic, "up0"); // Relay is high
    }
  }
}

  void subscribe_mqtt(const char* topic) {
    client.subscribe(topic);
    char *msg_connected = "Connected to";
    char notify[strlen(msg_connected) + strlen(topic) + 3];
    sprintf(notify, "%s: %s", msg_connected, topic);
    pub_mqtt(notify);
  }

  void reconnect() {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");

        char *msg_connected = "Connected to MQTT server";
        char notify[strlen(msg_connected) + strlen(mqtt_server) + 3];
        sprintf(notify, "%s: %s", msg_connected, mqtt_server);
        pub_mqtt(notify);

        subscribe_mqtt(inTopic);
        subscribe_mqtt(inTopic2);
        //      subscribe_mqtt(outTopic2);

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
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    //  detect a change in pysical switch
    if (last_sw_state != digitalRead(PIN_SW)) {
      if (digitalRead(PIN_SW) == 1) {
        if (digitalRead(REL_SONOFF) != LOW) {
          switch_off();
        }
      }
      else if (digitalRead(PIN_SW) == 0) {
        if (digitalRead(REL_SONOFF) != HIGH) {
          switch_on();
        }
      }
      last_sw_state = digitalRead(PIN_SW);
    }

    if (up_cmd == 1) {
//      Serial.println("upcmd=1");
      if (timeout_cmd < 10) {
//        Serial.print("to: ");
//        Serial.println(timeout_cmd);
        timeout_cmd ++;
        if (down_clearance == 1) {
//          Serial.println("dc=1");
          switch_on();
          up_cmd = 0;
          timeout_cmd = 0;
        }
      }
      else {
        timeout_cmd = 0;
        up_cmd =0 ;
      }
    }

    delay(200);

  }

