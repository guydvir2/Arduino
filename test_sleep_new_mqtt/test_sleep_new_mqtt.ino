#include <myESP32sleep.h>
// #include <myIOTesp32.h>
// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"
#include "EspMQTTClient.h"
// #include <NTPClient.h>
// #include <WiFiUdp.h>
// #include <WiFi.h>

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <WiFi.h>

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
// #include <TimeLib.h>
// #include <WiFiUdp.h>

// // NTP Servers:
// static const char ntpServerName[] = "us.pool.ntp.org";
// //static const char ntpServerName[] = "time.nist.gov";
// //static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
// //static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
// //static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

// const int timeZone = 1;     // Central European Time
// //const int timeZone = -5;  // Eastern Standard Time (USA)
// //const int timeZone = -4;  // Eastern Daylight Time (USA)
// //const int timeZone = -8;  // Pacific Standard Time (USA)
// //const int timeZone = -7;  // Pacific Daylight Time (USA)

// WiFiUDP Udp;
// unsigned int localPort = 8888;  // local port to listen for UDP packets

// time_t getNtpTime();
// void digitalClockDisplay();
// void printDigits(int digits);
// void sendNTPpacket(IPAddress &address);

#define DEVICE_TOPIC "ESP32_bad"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "solarPower"

EspMQTTClient client(
    "Xiaomi_D6C8",
    "guyd5161",
    "192.168.3.200", // MQTT Broker server ip
    "guy",           // Can be omitted if not needed
    "kupelu9e",      // Can be omitted if not needed
    DEVICE_TOPIC,    // Client name that uniquely identify your device
    1883             // The MQTT port, default to 1883. this line can be omitted
);

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("myHome/solarPower/ESP32_light", [](const String &payload) {
    Serial.println(payload);
  });

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  client.subscribe("mytopic/wildcardtest/#", [](const String &topic, const String &payload) {
    Serial.println(topic + ": " + payload);
  });

  // Publish a message to ""myHome/solarPower/ESP32_light"
  client.publish("myHome/solarPower/ESP32_light", "boot"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  // client.executeDelayed(5 * 1000, []() {
  //   client.publish("myHome/solarPower/ESP32_light", "This is a message sent 5 seconds later");
  // });
}

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 1
#define FORCE_AWAKE_TIME 30
#define NO_SLEEP_TIME 4
#define DEV_NAME "ESP32"

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);

void b4sleep()
{
  Serial.println("Going to Sleep");
  Serial.flush();
}
void startSleep_services()
{
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  go2sleep.no_sleep_minutes = NO_SLEEP_TIME;
  go2sleep.startServices();
}

int8_t timeZone = 2;
int8_t minutesTimeZone = 0;
const char *ntpServer = "pool.ntp.org";
// NTP.setInterval(63);
// NTP.setNTPTimeout (NTP_TIMEOUT);
NTP.begin(ntpServer, timeZone, true, minutesTimeZone);

void setup()
{
  Serial.begin(9600);

  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages();                                               // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater();                                                  // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("myHome/solarPower/ESP32_light/Avail", "offline"); // You can activate the retain flag by setting the third parameter to true
  timeClient.begin();

  // startSleep_services();
  // char a[50];
  // sprintf(a, "boot: %d", go2sleep.WakeStatus.bootCount);
  // Serial.println(a);

  // Serial.println("Starting UDP");
  // Udp.begin(localPort);
  // Serial.print("Local port: ");
  // // Serial.println(Udp.localPort());
  // Serial.println("waiting for sync");
  // setSyncProvider(getNtpTime);
  // setSyncInterval(300);
}

void loop()
{
  client.loop();
  Serial.print (NTP.getTimeDateString ()); Serial.print (" ");
  // Serial.println(timeClient.getFormattedTime());
  // go2sleep.wait_forSleep();
  delay(100);
}
