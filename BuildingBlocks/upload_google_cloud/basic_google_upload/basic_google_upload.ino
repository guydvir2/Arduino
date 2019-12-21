#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>

const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";
const char *host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
const char *fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String google_ID = "AKfycbwV7RGU3rEBcXIfCyKZpFCHUlhEWbyB19kd9WfLtCrF49Napbs"; // 

void setup()
{
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  client.setInsecure();
  Serial.print("Conntected to IP: ");
  Serial.println(WiFi.localIP());
  sendData(113, 125); // Send test data
}

void loop()
{
  sendData(1.23, 5.21);
  delay(1000);
}

void sendData(float x, float y)
{
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }
  if (client.verify(fingerprint, host))
  {
    Serial.println("certificate matches");
  }
  else
  {
    Serial.println("certificate doesn't match");
  }

  String string_x = String(x, DEC);
  String string_y = String(y, DEC);
  String url = "/macros/s/" + google_ID + "/exec?temperature=" + string_x + "&humidity=" + string_y;
  
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\""))
  {
    Serial.println("esp8266/Arduino CI successfull!");
  }
  else
  {
    Serial.println("esp8266/Arduino CI has failed");
  }

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}
