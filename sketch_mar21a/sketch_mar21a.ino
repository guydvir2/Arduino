
// ~~~~~~~ Sleep ~~~~~~~~~~~
#include "myESP32sleep.h"
#define SLEEP_TIME 1
#define FORCE_AWAKE_TIME 15
#define DEV_NAME "ESP32S"

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~ DAC & Solar Panel ~~~~~~~~~~~
#define USE_VMEASURE true
// #if USE_VMEASURE
struct voltReader
{
  int pin;
  float ADC_value;
  float v_divider;
  float correctF;
  float calc_value;
  float vlogic;
  float ADC_res;
};

voltReader battery = {39, 0.0, 0.5, 1.15, 0, 3.3, 4096};
voltReader solarPanel = {36, 0.0, 1 / 3, 1.1, 0};
const int Vsamples = 10;
float batADC_atBoot;

// #endif

void Vmeasure()
{
  battery.ADC_value = 0.0;
  solarPanel.ADC_value = 0.0;
  for (int a = 0; a < Vsamples; a++)
  {
    battery.ADC_value += analogRead(battery.pin);
    solarPanel.ADC_value += analogRead(solarPanel.pin);
    delay(50);
  }
  battery.ADC_value /= (float)Vsamples;
  battery.calc_value = battery.ADC_value / battery.ADC_res * battery.vlogic * battery.correctF / battery.v_divider;
  solarPanel.ADC_value /= (float)Vsamples;
  solarPanel.calc_value = solarPanel.ADC_value / solarPanel.ADC_res * solarPanel.vlogic * solarPanel.correctF / solarPanel.v_divider;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// +++++++++++ IFTT  ++++++++++++++
#define USE_IFTTT true
const char *server = "maker.ifttt.com";
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";
bool firstUpload = true;

void makeIFTTTRequest(float val1, float val2, char *val3)
{
  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0))
  {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected())
  {
    Serial.println("Failed to connect...");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);

  String jsonObject = String("{\"value1\":\"") + val1 + "\",\"value2\":\"" + val2 + "\",\"value3\":\"" + val3 + "\"}";

  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0))
  {
    delay(100);
  }
  if (!!!client.available())
  {
    Serial.println("No response...");
  }
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("\nclosing connection");
  client.stop();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  Serial.begin(9600);
  Serial.println("\n~~~~~~ Boot ~~~~~~");
  go2sleep.use_wifi = true;
  go2sleep.wifi_ssid = "Xiaomi_D6C8";
  go2sleep.wifi_pass = "guyd5161";
  go2sleep.startServices();
}

void loop()
{
  go2sleep.wait_forSleep();
  if (firstUpload){
    makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, "str");
    firstUpload = false;
  }
}
