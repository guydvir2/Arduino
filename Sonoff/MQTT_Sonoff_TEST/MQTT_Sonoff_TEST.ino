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
  createTopic(client_temp, clientID , inTopic);


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


void callback(char* topic, byte * payload, unsigned int length) {
  char incoming_msg[50];
  char *ans_status;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    incoming_msg[i] = (char)payload[i];
  }
  Serial.println();

  if (strcmp(incoming_msg,"status")==0) {
    status_reply(ans_status);
     Serial.print("ans_status: ");
    Serial.println(ans_status);
    
  }
}

void status_reply(char *answer) {
  char msg[4];
  int ans_rel = digitalRead(REL_SONOFF);
  int ans_led = digitalRead(LED_SONOFF);
  int ans_sw = digitalRead(PIN_SW);
  sprintf(msg,"%d%d%d",ans_rel,ans_sw,ans_led);
  Serial.println(msg);
  strcpy(answer, msg);
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

}



