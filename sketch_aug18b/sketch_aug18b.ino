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
char *ver = "1.1";

const char* ssid = "HOTBOX-Darabi";
const char* password = "75999276";
const char* mqtt_server = "guydvir.noip.me";

int last_sw_state = 0;
int up_cmd = 0;
int down_cmd = 0;
int down_clearance = 0;
int up_clearance = 0;
int timeout_cmd = 0 ;

const char *clientID = "Sonoff1";
const char *dev_direction = "up"; // "UP or "DOWN"

const char *client_temp = "HomePi/Dvir/Windows/";
const char *outTopic = "HomePi/Dvir/Messages";
const char* inTopic2 = "HomePi/Dvir/Windows/All";
//char inTopic[50];
char *inTopic = "HomePi/Dvir/Windows/Sonoff1";
char *inTopicDown = "HomePi/Dvir/Windows/Sonoff1/down";

int counter = 0;
int counter2 = 0;

WiFiClient espClient;
PubSubClient client(espClient);


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

void subscribe_mqtt(const char* topic) {
  client.subscribe(topic);
  char *msg_connected = "Connected to";
  char notify[strlen(msg_connected) + strlen(topic) + 3];
  sprintf(notify, "%s: %s", msg_connected, topic);
  pub_mqtt(notify);
}

void pub_mqtt(char* msg1) {
  char outMessage[strlen(inTopic) + strlen(msg1) + 3];
  sprintf(outMessage, "[%s] %s", inTopic, msg1);
  client.publish(outTopic, outMessage);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("up_dev")) {
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  start_wifi();

  // start mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
  if (strcmp(dev_direction, "up") == 0) {
    if (strcmp(incoming_msg, "up_cmd0") == 0) {
      up_cmd = 1;
      up_cmd0();

    }
    else if (strcmp(incoming_msg, "guy") == 0) {
      up_clearance = 10;
    }
  }
}

void up_cmd0() {
  char res [20];
  sprintf (res, "up_cmd0_res:%i", counter2);
  client.publish(inTopic, "guy");
  delay(100);
}

int verify_upcmd0() {
  if (up_clearance == 10) {
    return 1;
  }
}

void reset_flags() {
  up_cmd = 0;
  up_clearance = 0;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  counter++;
  counter2 = counter2 + 2;
  Serial.println(counter);
  delay(1000);

  if (up_cmd == 1) {
    if (verify_upcmd0() == 1) {
      Serial.println("OK");
      reset_flags();
    }
  }


}

