#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid = "Xiaomi_D6C8";
const char* password = "guyd5161";

int timezone = 3;
int dst = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop() {
  time_t now = time(15200000);
  Serial.println(ctime(&now));
//    Serial.println(ctime(15200000));

  delay(1000);
}
