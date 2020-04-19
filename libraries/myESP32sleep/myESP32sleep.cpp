#include "Arduino.h"
#include "myESP32sleep.h"

#define drift_ArraySize 3
#define zeroval 999.0

RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR float driftRTC = 0;
RTC_DATA_ATTR long clock_beforeSleep = 0;
RTC_DATA_ATTR float driftsArray_RTC[drift_ArraySize];

// ~~~~~~~ EEPROM ~~~~~~~~~
int esp32Sleep::getEEPROMvalue(byte i)
{
  int eeprom_drift = EEPROM.read(i) * pow(-1, EEPROM.read(i + 1));
  return eeprom_drift;
}
void esp32Sleep::saveEEPROMvalue(int val, byte i)
{
  EEPROM.write(i, abs(val));
  if (val < 0)
  {
    EEPROM.write(i + 1, 1);
    EEPROM.commit();
  }
  else
  {
    EEPROM.write(i + 1, 2);
    EEPROM.commit();
  }
}
void esp32Sleep::start_eeprom(byte i)
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    // Serial.println("Fail to load EEPROM");
  }
}

// ~~~~~~~~ Wifi & NTP ~~~~~~
void esp32Sleep::startNTP(const int gmtOffset_sec, const int daylightOffset_sec, const char *ntpServer)
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
bool esp32Sleep::startWifi()
{
  long beginwifi = millis();
  WiFi.begin(wifi_ssid, wifi_pass);
  while (WiFi.status() != WL_CONNECTED && millis() - beginwifi < 30000)
  {
    delay(200);
    // Serial.print(".");
  }
  // Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    // Serial.println(WiFi.localIP());
    startNTP();
    return 1;
  }
  else
  {
    // Serial.println("Failed connect to wifi");
    return 0;
  }
}
void esp32Sleep::getTime()
{
  int a = 0;
  while (a < 3)
  {
    if (getLocalTime(&_timeinfo))
    {
      delay(100);
      time(&_epoch_time);
    }
    a++;
  }
}
void esp32Sleep::Avg_Array_zeroing()
{
  if (bootCounter == 0)
  {
    for (int a = 0; a < drift_ArraySize; a++)
    {
      driftsArray_RTC[a] = zeroval;
    }
  }
}

// void esp32Sleep::onConnectionEstablished()
// {
//   // Subscribe to "mytopic/test" and display received message to Serial
//   espmqtt_client.subscribe("mytopic/test", [](const String & payload) {
//     Serial.println(payload);
//   });

//   // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
//   espmqtt_client.subscribe("mytopic/wildcardtest/#", [](const String & topic, const String & payload) {
//     Serial.println(topic + ": " + payload);
//   });

//   // Publish a message to "mytopic/test"
//   espmqtt_client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

//   // Execute delayed instructions
//   espmqtt_client.executeDelayed(5 * 1000, []() {
//     espmqtt_client.publish("mytopic/test", "This is a message sent 5 seconds later");
//   });
// }

// ~~~~~~~ Sleep & Drift calcs ~~~

void esp32Sleep::update_driftArray(float lastboot_drift)
{
  float sum_avg = 0.0;

  for (int a = drift_ArraySize - 1; a > 0; a--)
  {
    driftsArray_RTC[a] = driftsArray_RTC[a - 1];
  }

  driftsArray_RTC[0] = lastboot_drift;

  if (bootCounter > drift_ArraySize)
  {
    for (int a = 0; a < drift_ArraySize; a++)
    {
      sum_avg += driftsArray_RTC[a];
    }
    driftRTC = sum_avg / (float)drift_ArraySize;
  }
  // Serial.print("Mean driftRTC: ");
  // Serial.println(driftRTC);
}
void esp32Sleep::driftUpdate(float lastboot_drift, byte cell)
{
  // if (bootCounter <= drift_ArraySize + 2)
  // {
  //   driftRTC += lastboot_drift;
  //   if (bootCounter > 2) // first 2 boots will not enter to avg_array
  //   {
  //     driftsArray_RTC[bootCounter - 3] = lastboot_drift;
  //   }
  // }
  // else
  // {
  //   float sum_avg = 0.0;
  //   for (int a = drift_ArraySize - 1; a > 0; a--)
  //   {
  //     driftsArray_RTC[a] = driftsArray_RTC[a - 1];
  //     sum_avg += driftsArray_RTC[a];
  //   }
  //   driftsArray_RTC[0] = lastboot_drift;
  //   sum_avg += lastboot_drift;
  //   driftRTC += sum_avg / (float)drift_ArraySize;

  //   if (bootCounter % 10 == 0)
  //   {
  //     saveEEPROMvalue((int)driftRTC, 0);
  //     Serial.println("DRIFT SAVED TO EEPROM");
  //   }
  // }
}

void esp32Sleep::new_driftUpdate(float lastboot_drift, byte cell)
{
  const float driftFactor = -0.006111;
  const float drift_tolerance = 0.25;
  const float nomin_drift = driftFactor * (_deepsleep_time * 60.0);
  const float max_drift = nomin_drift * (1 - drift_tolerance); // bigger neg number
  const float min_drift = nomin_drift * (1 + drift_tolerance); // lesser neg number

  if (driftRTC + lastboot_drift < max_drift && driftRTC + lastboot_drift > min_drift)
  {
    driftRTC += lastboot_drift;
    // Serial.println("drift value added");
  }
  else if (driftRTC + lastboot_drift > min_drift)
  {
    driftRTC = min_drift;
    // Serial.println("drift value corrected to min ");
  }
  else if (driftRTC + lastboot_drift < max_drift)
  {
    driftRTC = max_drift;
    // Serial.println("drift value corrected to max");
  }
  update_driftArray(driftRTC);
}

int esp32Sleep::calc_nominal_sleepTime()
{
  int nominal_nextSleep = 0;
  char tt[100];
  getTime();

  if (_timeinfo.tm_year >= 120)
  {
    nominal_nextSleep = _deepsleep_time * 60 - (_timeinfo.tm_min * 60 + _timeinfo.tm_sec) % (_deepsleep_time * 60);
    clock_beforeSleep = _epoch_time;                      // RTC var
    clock_expectedWake = _epoch_time + nominal_nextSleep; // RTC var
  }
  else // fail to obtain clock
  {
    nominal_nextSleep = _deepsleep_time * 60;
  }

  sprintf(tt, "wake_Duration: [%.2fs]; back2Sleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift_correct: [%.1f sec]",
          millis() / 1000.0, _timeinfo.tm_hour, _timeinfo.tm_min, _timeinfo.tm_sec, nominal_nextSleep, driftRTC);
  strcat(wake_sleep_str, tt);
  return nominal_nextSleep;
}
esp32Sleep::esp32Sleep(int deepsleep, int forcedwake, char *devname)
    : mqttClient(espClient)
{
  _deepsleep_time = deepsleep;
  _forcedwake_time = forcedwake;
  dev_name = devname;
}
bool esp32Sleep::startServices(char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker)
{
  start_eeprom();
  // Avg_Array_zeroing();
  mqtt_server = mqtt_broker;
  user = mqtt_user;
  passw = mqtt_passw;
  wifi_ssid = ssid;
  wifi_pass = password;

  if (use_wifi)
  {
    _wifi_status = startWifi();
    if (_wifi_status)
    {
      check_awake_ontime();
    }
    return 1;
  }
  else
  {
    sprintf(wake_sleep_str, "Fail to obtain WiFi");
    return 0;
  }
}
void esp32Sleep::startMQTT()
{
  bool stat = false;
  mqttClient.setServer(mqtt_server, 1883);
  stat = true;
  Serial.println("MQTT SERVER: ");
  Serial.println(mqtt_server);

  // Set callback function
  if (stat)
  {
    mqttClient.setCallback(std::bind(&esp32Sleep::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    subscribeMQTT();
  }
  else
  {
    Serial.println("Not connected to MQTT server");
  }
}
bool esp32Sleep::subscribeMQTT()
{
  static long lastReconnectAttempt = 0;
  
  if (!mqttClient.connected())
  {
    long now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;

        Serial.println("have wifi, entering MQTT connection");
        Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      char tempname[15];
      sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());

      if (mqttClient.connect(tempname, user, passw, _availTopic, 0, true, "offline"))
      {
        for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++)
        {
          if (strcmp(topicArry[i], "") != 0)
          {
            mqttClient.subscribe(topicArry[i]);
          }
        }

        if (useSerial)
        {
          Serial.println("connected");
        }
        if (firstRun)
        {
          pub_log("<< PowerON Boot >>");
          if (!useResetKeeper)
          {
            firstRun = false;
            mqtt_detect_reset = 0;
            notifyOnline();
          }
          else
          { // using reset keeper
            mqttClient.publish(_availTopic, "resetKeeper", true);
          }
        }
        else
        { // not first run
          notifyOnline();
        }
        return 1;
      }
      else
      { // fail to connect MQTT
        if (useSerial)
        {
          Serial.print("failed, rc=");
          Serial.println(mqttClient.state());
        }
        return 0;
      }
    }
  }
  else
  {
    return 1;
  }
}
void esp32Sleep::sleepNOW(float sec2sleep)
{
  if (_use_extfunc)
  {
    _runFunc();
  }
  esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
void esp32Sleep::check_awake_ontime(int min_t_avoidSleep)
{
  delay(3000);
  getTime();
  bootCounter++;

  sprintf(sys_presets_str, "deviceName:[%s]; SleepTime: [%d min]; Forced-aWakeTime: [%d sec]", dev_name, _deepsleep_time, _forcedwake_time);
  sprintf(wake_sleep_str, "Boot#: [%d]; Wake_Clock: [%02d:%02d:%02d]; ", bootCounter, _timeinfo.tm_hour, _timeinfo.tm_min, _timeinfo.tm_sec);

  if (_timeinfo.tm_year >= 120)
  {
    if (clock_beforeSleep > 0)
    {
      int wake_diff = _epoch_time - clock_expectedWake;        // not first boot
      int t_delta = wake_diff - (int)(round(millis() / 1000)); // diff between calc wake clock and current time

      char tt[100];
      sprintf(tt, "Woke_after: [%d sec]; wake_Drift: [%d sec]; ", _epoch_time - clock_beforeSleep, t_delta);
      strcat(wake_sleep_str, tt);
      new_driftUpdate(t_delta, 0);

      if (t_delta < 0 && wake_diff < 0)
      {
        if (abs(wake_diff) <= min_t_avoidSleep)
        {
          // wake SHORT while before time, wait using a delay func.
          sprintf(tt, "Pause_wakeBeforeTime: [%d sec]; ", abs(wake_diff));
          strcat(wake_sleep_str, tt);
          delay(1000 * abs(wake_diff));
        }
        else
        {
          // wake before time, goto sleep.
          sleepNOW(abs(wake_diff));
        }
      }
    }
    else if (getEEPROMvalue() > 0)
    {
      driftRTC = (int)getEEPROMvalue();
    }
    else
    {
      saveEEPROMvalue(0, 0);
    }
  }
}
void esp32Sleep::wait_forSleep()
{
  // when forced awake time will be over, ESP will go to slepp
  if (millis() >= _forcedwake_time * 1000)
  {
    if (_wifi_status)
    {
      sleepNOW(calc_nominal_sleepTime() - driftRTC);
    }
    else
    {
      sleepNOW(_deepsleep_time * 60);
    }
  }
}
void esp32Sleep::run_func(cb_func cb)
{
  _runFunc = cb;
  _use_extfunc = true;
}
