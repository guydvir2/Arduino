
// ~~~~~~~ Sleep ~~~~~~~~~~~
#include "myESP32sleep.h"
#define SLEEP_TIME 30
#define FORCE_AWAKE_TIME 20
#define DEV_NAME "ESP32-S" //"ESP32light" //
esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~ DAC & Solar Panel ~~~~~~~~~~~
#define USE_BAT_SOLAR false
#define USE_IFTTT false
#define USE_VMEASURE false
#define VBAT_ADC_PIN 35
#define VSOLAR_ADC_PIN 34

struct voltReader
{
  int pin;
  int ADC_value;
  float v_divider;
  float correctF;
  float calc_value;
  float vlogic;
  int ADC_res;
};

voltReader battery = {VBAT_ADC_PIN, 0, 0.5, 1, 0.0, 3.3, 4096};
voltReader solarPanel = {VSOLAR_ADC_PIN, 0, 0.5, 1, 0.0, 3.3, 4096};

void Vmeasure()
{
  const int Vsamples = 4;
  battery.ADC_value = 0;
  solarPanel.ADC_value = 0;

  for (int a = 0; a < Vsamples; a++)
  {
    battery.ADC_value += analogRead(battery.pin);
    solarPanel.ADC_value += analogRead(solarPanel.pin);
    delay(50);
  }

  battery.calc_value = (((float)battery.ADC_value / (float)Vsamples) / (float)battery.ADC_res) * battery.vlogic * battery.correctF / battery.v_divider;
  solarPanel.calc_value = (((float)solarPanel.ADC_value / (float)Vsamples) / (float)solarPanel.ADC_res) * solarPanel.vlogic * solarPanel.correctF / solarPanel.v_divider;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// +++++++++++ IFTT  ++++++++++++++
const char *server = "maker.ifttt.com";
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";

void makeIFTTTRequest(char *val1, char *val2, char *val3)
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

void b4sleep()
{
  char tt[50];
  sprintf(tt, "BAT: [%.2fv], Solar: [%.2fv]", battery.calc_value, solarPanel.calc_value);
#if USE_IFTTT
  makeIFTTTRequest(go2sleep.wake_sleep_str, go2sleep.sys_presets_str, tt);
#endif
}
void setup()
{
  Serial.begin(9600);
  Serial.println("\n~~~~~~ Boot ~~~~~~");

  if (USE_BAT_SOLAR)
  {
    Vmeasure();
  }
  go2sleep.run_func(b4sleep);
  go2sleep.startServices();
}

void loop()
{
  go2sleep.wait_forSleep();
  delay(100);
}
