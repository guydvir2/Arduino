#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// GPIOs on SONOFF board
int LED_SONOFF = 13;
int REL_SONOFF = 12;
int PIN_SW = 14;

const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
const char* mqtt_server = "192.168.2.113";
int last_sw_state = 0;

WiFiClient espClient;
PubSubClient client(espClient);


char msg[50];
const char* clientID = "Sonoff";
char *client_temp = "HomePi/Dvir/Windows/";
const char* outTopic = "HomePi/Dvir/Messages";
const char* outTopic2 = "HomePi/Dvir/Windows/ESP32";
const char* inTopic [strlen(client_temp) + 3];
const char* inTopic2 = "HomePi/Dvir/Windows/All";

String clock;

void start_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  for (int i=0; i<3; i++){
    digitalWrite(LED_SONOFF,LOW);
    delay(50); 
    digitalWrite(LED_SONOFF,HIGH);
    delay(50); 
  }
}

void get_time(){
  client.publish(outTopic2,"time");
}
void setup() {
  Serial.begin(115200); // start serial service
  delay(10);

  // define sonoff gpios
  pinMode(LED_SONOFF, OUTPUT);
  pinMode(REL_SONOFF, OUTPUT);
  pinMode(PIN_SW, INPUT_PULLUP); // GPIO14 has a pullup resistor
  digitalWrite(LED_SONOFF,HIGH); // LED high is OFF
  digitalWrite(REL_SONOFF,LOW); 

  start_wifi(); // start wifi service


  // start mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  
//  run PowerOnBit
  PBit();
}

void PBit() {
  switch_on();
  delay(500);
  switch_off();
  delay(500);
}

void switch_on() {
  digitalWrite(REL_SONOFF,HIGH);
  digitalWrite(LED_SONOFF, LOW); // LED ON
  pub_mqtt("ON");
}

void switch_off() {
  digitalWrite(REL_SONOFF,LOW);
  digitalWrite(LED_SONOFF, HIGH); // LED OFF
  pub_mqtt("OFF");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    switch_on();   
  } 
  if ((char)payload[0] == '0') {
    switch_off();
    Serial.print("payload: ");
    Serial.println(payload[0]);
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
      subscribe_mqtt(outTopic2);

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
  if (last_sw_state != digitalRead(PIN_SW)){
    if (digitalRead(PIN_SW)==1) {
      if (digitalRead(REL_SONOFF)!=LOW) {
        switch_off();
      }
    }
    else if (digitalRead(PIN_SW)==0) {
      if (digitalRead(REL_SONOFF)!=HIGH){
       switch_on();
      }
    }
    last_sw_state = digitalRead(PIN_SW);
  }
  
   delay(100);
}

