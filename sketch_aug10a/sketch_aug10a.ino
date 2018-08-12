/*
Blink LED - Sonoff Basic
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Pin config
int LED_SONOFF = 13;
int REL_SONOFF = 12 ;
int PIN_SW = 14;

int BLINK_DURATION = 2000;

const char* ssid     = "HomeNetwork_2.4G";
const char* password = "guyd5161";
const char* mqtt_server = "192.168.2.113";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const char* outTopic = "HomePi/Dvir/Messages";
const char* inTopic = "HomePi/Dvir/Windows/SONOFF";

void wifi_conn() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(LED_SONOFF, OUTPUT);
  pinMode(REL_SONOFF, OUTPUT);
  pinMode(PIN_SW, INPUT);

  wifi_conn();

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
//  if (digitalRead(PIN_SW)==HIGH) {
//    Serial.println("UP");
//  }
//  digitalWrite(LED_SONOFF, LOW); // LOW will turn on the LED
//  digitalWrite(REL_SONOFF, digitalRead(LED_SONOFF));
//  Serial.println(digitalRead(REL_SONOFF));
//  delay(BLINK_DURATION);
//  digitalWrite(LED_SONOFF, HIGH); // HIGH will turn off the LED
//  digitalWrite(REL_SONOFF, digitalRead(LED_SONOFF));
//  Serial.println(digitalRead(REL_SONOFF));
//  delay(BLINK_DURATION);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
  
  
}

