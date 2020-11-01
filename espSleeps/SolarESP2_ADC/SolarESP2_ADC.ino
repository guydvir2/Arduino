#include <myIOTesp32.h>
#include <myESP32sleep.h>
#include <Adafruit_ADS1015.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#define VER "ESP32_v1.3"
#define USE_VMEASURE true
#define USE_SLEEP true
#define USE_ADS true
// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32_12v"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "solarPower"
#define MQTT_TELEGRAM "myHome/Telegram"
#define MQTT_EXT_TOPIC MQTT_PREFIX "/" MQTT_GROUP "/" DEVICE_TOPIC "/" \
                                   "debug"
#define USE_SERIAL true
#define USE_OTA true
#define USE_WDT false
#define USE_EXT_TOPIC true
#define USE_RESETKEEPER true

myIOT32 iot(DEVICE_TOPIC);

void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.useExtTopic = USE_EXT_TOPIC;
  iot.ext_mqtt_cb = ext_MQTT;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.telegramServer, MQTT_TELEGRAM);
  strcpy(iot.extTopic, MQTT_EXT_TOPIC);

  iot.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 30
#define FORCE_AWAKE_TIME 10
#define NO_SLEEP_TIME 4
#define DEV_NAME DEVICE_TOPIC
bool no_sleep_flag = false;

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
void b4sleep()
{
  postWake();
}
void startSleep_services()
{
  go2sleep.debug_mode = false;
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  go2sleep.no_sleep_minutes = NO_SLEEP_TIME;
  go2sleep.startServices();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// // ~~~~~ Voltage Measurements ~~~~
// const int bat_voltagePin = 35;
// const int solar_voltagePin = 36;
// const int ADC_res = 4095;
// const float vcc = 3.3;
const float v_div_bat = 4.0;   //1.5;//1.33;// for 6v panel :1.5;
const float v_div_solar = 4.0; // for 6v panel: 2.0; // // 5.0
// float bat_volt = 0.0;
// float solar_volt = 0.0;

// float measure_voltage(const int &pin, float ratio = 1.0)
// {
//   float measure = 0.0;
//   float correction_factor = 0;
//   float non_linear_margin = 0.1;
//   byte sample = 10;

//   for (int i = 0; i < sample; i++)
//   {
//     measure += analogRead(pin);
//     delay(10);
//   }

//   float temp_calc = (measure / (float)sample) / (float)ADC_res;
//   Serial.println(temp_calc);
//   if (temp_calc > non_linear_margin && temp_calc < 1 - non_linear_margin)
//   {
//     correction_factor = 0.2;
//   }
//   else
//   {
//     correction_factor = 0.0;
//   }
//   measure = ((measure / (float)sample) / (float)ADC_res) * ratio * vcc + correction_factor * ratio;
//   return measure;
// }
// void start_voltageMeasure()
// {
//   pinMode(bat_voltagePin, INPUT);
//   pinMode(solar_voltagePin, INPUT);

//   bat_volt = measure_voltage(bat_voltagePin, v_div_bat);
//   solar_volt = measure_voltage(solar_voltagePin, v_div_solar);
// }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void checkWake_topic()
{
  if (strcmp(iot.mqtt_msg.msg, "m") == 0)
  {
    start_maintainance();
    sprintf(iot.mqtt_msg.msg, "");
  }
}
String create_wakeStatus()
{
  StaticJsonDocument<300> doc;
  // Constansts
  doc["deviceName"] = go2sleep.WakeStatus.name;
  doc["bootCount"] = go2sleep.WakeStatus.bootCount;
  doc["forcedAwake"] = go2sleep.WakeStatus.awake_duration;

  // Per wake cycle
  doc["sleepDuration"] = go2sleep.sleepduration;
  doc["Wake"] = go2sleep.WakeStatus.wake_clock;
  doc["SleepStart"] = go2sleep.WakeStatus.startsleep_clock;
  doc["nextWake"] = go2sleep.WakeStatus.nextwake_clock;
  doc["RTCdrift"] = go2sleep.WakeStatus.drift_err;
  doc["WakeErr"] = go2sleep.WakeStatus.wake_err;

  String output;
  serializeJson(doc, output);
  return output;
}
void postWake()
{
  char a[250];
  String output = create_wakeStatus();
  output.toCharArray(a, output.length() + 1);
  iot.pub_ext(a, true);
}
void start_maintainance()
{
  no_sleep_flag = true;
  iot.pub_ext("maintainance_started", true);
  iot.pub_tele("Maintainance");
  iot.pub_log("Maintainance Started");
}

void ext_MQTT(char *incoming_msg)
{
  char msg[200];
  char msg2[20];

  if (strcmp(incoming_msg, "status") == 0)
  {
    // giveStatus(msg);
    iot.pub_msg("msg");
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver #1: Code: [%s],lib: [%s]", VER, iot.ver);
    iot.pub_msg(msg);

    sprintf(msg, "ver #2: WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d]",
            USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "maintainance") == 0)
  {
    sprintf(msg, "Maintainance: forced No-sleep [%d minutes] started", NO_SLEEP_TIME);
    start_maintainance();
    iot.pub_msg(msg);
    iot.pub_tele(msg);
  }
  else if (strcmp(incoming_msg, "sleep_p") == 0)
  {
    sprintf(msg, "sleep_Parameters: SleepDuration[%d minuntes], forcedWakeTime[%d sec], forced No-sleep [%d minutes]",
            SLEEP_TIME, FORCE_AWAKE_TIME, NO_SLEEP_TIME);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0)
  {
    sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg);
    sprintf(msg, "Help: Commands #2 - [sleep_p, sleepNow, maintainance(onWake Topic)]");

    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "sleepNow") == 0)
  {
    b4sleep();
    go2sleep.sleepNOW(SLEEP_TIME * 60);
  }
}

// +++++++++++ IFTT  ++++++++++++++
// const char *server = "maker.ifttt.com";
// const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";

// template <typename T1, typename T2, typename T3>
// void makeIFTTTRequest(T1 val1, T2 val2, T3 val3)
// {
//   Serial.print("Connecting to ");
//   Serial.print(server);

//   WiFiClient client;
//   int retries = 5;
//   while (!!!client.connect(server, 80) && (retries-- > 0))
//   {
//     Serial.print(".");
//   }
//   Serial.println();
//   if (!!!client.connected())
//   {
//     Serial.println("Failed to connect...");
//   }

//   Serial.print("Request resource: ");
//   Serial.println(resource);

//   String jsonObject = String("{\"value1\":\"") + val1 + "\",\"value2\":\"" + val2 + "\",\"value3\":\"" + val3 + "\"}";

//   client.println(String("POST ") + resource + " HTTP/1.1");
//   client.println(String("Host: ") + server);
//   client.println("Connection: close\r\nContent-Type: application/json");
//   client.print("Content-Length: ");
//   client.println(jsonObject.length());
//   client.println();
//   client.println(jsonObject);

//   int timeout = 5 * 10; // 5 seconds
//   while (!!!client.available() && (timeout-- > 0))
//   {
//     delay(100);
//   }
//   if (!!!client.available())
//   {
//     Serial.println("No response...");
//   }
//   while (client.available())
//   {
//     Serial.write(client.read());
//   }

//   Serial.println("\nclosing connection");
//   client.stop();
// }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
Adafruit_ADS1115 ads;

int16_t adc_bat, adc_solar, adc_amp;
const float ADC_convFactor = 0.1875;
const float solarVoltageDiv = 0.2;

void measureADS()
{
  adc_bat = ads.readADC_SingleEnded(0);
  adc_solar = ads.readADC_SingleEnded(1);
  adc_amp = ads.readADC_SingleEnded(2);
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  startIOT_services();
  ads.begin();

  char a[50];
  char b[80];

#if USE_SLEEP
  startSleep_services();
  sprintf(a, "Boot: [#%d]", bootCounter);
#endif

#if USE_VMEASURE
  measureADS();
  sprintf(b, "Wake: ADC_bat[%.2fv]; ADC_Solar[%.2fv], ADC_Current[%.2f]", 
  adc_bat * ADC_convFactor / 1000.0, (adc_solar * ADC_convFactor / 1000.0)/solarVoltageDiv,
  adc_amp);
  Serial.println(adc_solar);
#endif
  iot.pub_log(b);
}

void loop()
{
  if (go2sleep.wait_forSleep(iot.networkOK, no_sleep_flag))
  {
    checkWake_topic();
    iot.looper();
  }
  delay(100);
}
