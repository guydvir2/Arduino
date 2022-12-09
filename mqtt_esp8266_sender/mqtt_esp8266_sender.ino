#if defined ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <PubSubClient.h>

#define ROLE 0 /* 0: sender; 1: Reciever */
#define SEND_INTERVAL_MS 200

const char *ssid = "dvirz_iot";
const char *password = "GdSd13100301";
const char *mqtt_server = "192.168.2.100";
const char *outTopic = "myHome/test/ESP8266";
const char *inTopic = "myHome/test2";
const char *mqtt_msg = "status";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.print(" ->err: ");
  int c=(int)SEND_INTERVAL_MS - (int)(millis() - lastMsg);
  Serial.println(c);
  lastMsg = millis();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "guy", "kupelu9e"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "hello world");
      // ... and resubscribe
      client.subscribe(inTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  if (ROLE == 1)
  {
    client.setCallback(callback);
  }
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (ROLE == 0)
  {
    unsigned long now = millis();
    if (now - lastMsg > SEND_INTERVAL_MS)
    {
      ++value;
      snprintf(msg, MSG_BUFFER_SIZE, mqtt_msg);
      // sprintf(msg, "msg_#%d_%d", value, SEND_INTERVAL_MS - (now - lastMsg));
      lastMsg = now;
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(outTopic, msg);
    }
  }
}
