#include <Arduino.h>
#include <EEPROM.h>
#include <myDisplay.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ~~ Temp Sensor ~~
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature TempSensor(&oneWire);
DeviceAddress tempSensorAdress;

#define pressed LOW

myLCD LCDdisplay(16, 2);

#define MAX_T 32.0
#define MIN_T 20.0
#define INC_T 0.5
#define START_T 28.0
#define STOP_T 25.0
#define MAX_ON_TIME 60 * 5   //max minutes ON
#define TIMEOUT_AFTER_ON 60  // minutes timeout after OFF between re-ON
#define MANUAL_ON_TIMEOUT 60 // manual ON minutes

int systemState = 0; // 0:Off, 1:on 2:SETUP
float temperature_store[2] = {START_T, STOP_T};
int store_pointer = 0;
float tempC = 0;

const int buts = 2;
const int button1Pin = 2;
const int button2Pin = 3;
const int but0LEDPin = 5;
const int but1LEDPin = 6;
const int *butPins[] = {&button1Pin, &button2Pin};
const int *ledPins[2] = {&but0LEDPin, &but1LEDPin};

float press_time = 0.0;
const float longPress = 1.0;
const float timeout_longPress = 5.0;
unsigned long lastPress_clock = 0;
unsigned long start_run_clock = 0;
unsigned long end_run_clock = 0;

// ~~~~~~~ System States ~~~~~~
void turnSystem_on(int timeout)
{
  systemState = 1;
  start_run_clock = millis();
  end_run_clock = start_run_clock + 1000 * timeout * 60;
  //Switch on Relay
  // Buzz ON
}
void turnSystem_off()
{
  systemState = 0;
  start_run_clock = 0;
  end_run_clock = 0;
  //Switch off Relay
  //Buzz OFF
}
void start_manualOP()
{
  Serial.println("Start_manual_OP");
  turnSystem_on(MANUAL_ON_TIMEOUT);
}
void end_manualOP()
{
  Serial.println("END_manual_OP");
  turnSystem_off();
}
void start_setupMode()
{
  Serial.println("Enter SetUpMode");
  Serial.print("Start Temp: ");
  Serial.println(temperature_store[0]);
  Serial.print("Stop Temp: ");
  Serial.println(temperature_store[1]);

  store_pointer = 0;
  systemState = 2;
}
void end_setupMode()
{
  Serial.println("Exit SetUpMode");
  systemState = 0;
  store_pointer = 0;
}

// ~~~ Button Manipulations ~~~
void inc_value()
{
  Serial.println("increase value");
  if (temperature_store[store_pointer] + INC_T <= MAX_T)
  {
    temperature_store[store_pointer] += INC_T;
  }
  else
  {
    temperature_store[store_pointer] = MIN_T;
  }
  Serial.print("temp store #");
  Serial.print(store_pointer);
  Serial.print(": ");
  Serial.println(temperature_store[store_pointer]);
}
void dec_value()
{
  Serial.println("dec value");
  if (temperature_store[store_pointer] - INC_T >= MIN_T)
  {
    temperature_store[store_pointer] -= INC_T;
  }
  else
  {
    temperature_store[store_pointer] = MIN_T;
  }
  Serial.print("temp store #");
  Serial.print(store_pointer);
  Serial.print(": ");
  Serial.println(temperature_store[store_pointer]);
}
void readButton(const int &pin)
{
  while (digitalRead(pin) == pressed && press_time <= longPress)
  {
    if (lastPress_clock == 0)
    {
      lastPress_clock = millis();
    }
    else
    {
      press_time = (float)((millis() - lastPress_clock) / 1000.0);
    }
  }

  if (press_time > 0)
  {
    if (pin == *butPins[0])
    {
      // Serial.println("Button No.1 was pressed");
      if (press_time > longPress && press_time < timeout_longPress && systemState == 0)
      {
        start_manualOP();
      }
      else if (press_time < longPress && systemState == 2)
      {
        inc_value();
      }
    }
    else if (pin == *butPins[1])
    {
      // Serial.println("Button No.2 was pressed");
      if (press_time > longPress && systemState == 1)
      {
        end_manualOP();
      }
      else if (press_time > longPress && systemState == 0)
      {
        start_setupMode();
      }
      else if (press_time > longPress && systemState == 2)
      {
        writeEEPROMval(store_pointer, temperature_store[store_pointer]);
        if (store_pointer == 0)
        {
          store_pointer++;
        }
        else if (store_pointer == 1)
        {
          end_setupMode();
        }
      }
      else if (press_time < longPress && systemState == 2)
      {
        dec_value();
      }
    }
    lastPress_clock = 0;
    press_time = 0;
  }
}
void read_butLoop()
{
  for (int a = 0; a < buts; a++)
  {
    readButton(*butPins[a]);
  }
}

// ~~~~~~ Display ~~~~~~
void startLEDdisplay()
{
  LCDdisplay.start();
}
void displayLED()
{
  char a[20];
  LCDdisplay.clear();
  sprintf(a, "%.1f", (float)(millis() / 1000.0));
  LCDdisplay.CenterTXT("Running Time", a);
  delay(500);
}

// ~~~~~~ EEPROM ~~~~~~
void check_EEPROMVal()
{
  int arr[4] = {(int)temperature_store[0], 0, (int)temperature_store[1], 0};
  for (int a = 0; a < 4; a++)
  {
    if (EEPROM.read(a) == 255)
    {
      EEPROM.write(a, arr[a]);
      Serial.println(EEPROM.read(a));
    }
  }
}
void read_EEPROMval(int i)
{
  Serial.print("Stored value: #");
  Serial.print(i);
  Serial.print(" ");
  Serial.print(EEPROM.read(2 * i));
  Serial.print(".");
  Serial.println(EEPROM.read(2 * i + 1));
}
void writeEEPROMval(int i, float val)
{
  int dec_val = val * 10 - (int)val * 10;
  EEPROM.write(2 * i, (int)val);
  EEPROM.write(2 * i + 1, dec_val);

  Serial.print("Value Saved ");
  Serial.print(EEPROM.read(2 * i));
  Serial.print(".");
  Serial.println(EEPROM.read(2 * i + 1));
}

void initHW()
{
  for (int a = 0; a < buts; a++)
  {
    pinMode(*butPins[a], INPUT_PULLUP);
    pinMode(*ledPins[a], OUTPUT);
  }
}

// ~~ Temperature functions ~~
void tempSensor_init()
{
  TempSensor.begin();
  if (!TempSensor.getAddress(tempSensorAdress, 0))
    Serial.println("Unable to find address for Device 0");
  Serial.println();

  if (TempSensor.getDeviceCount() > 0)
  {
    Serial.println("Temperature sensor Found OK");
  }
  else
  {
    Serial.println("Temperature sensor not-Found");
  }
}
void getTemperature()
{
  static unsigned long lastRead = 0;
  int read_interval = 10; //seconds
  if (millis() - lastRead > read_interval * 1000)
  {
    TempSensor.requestTemperatures();
    tempC = TempSensor.getTempC(tempSensorAdress);
    Serial.print("Temp C: ");
    Serial.println(tempC);
    lastRead = millis();
  }
}
void check_temp_loop()
{
  getTemperature();
  if (tempC >= temperature_store[0] && tempC >= temperature_store[1])
  {
    if (systemState == 0)
    {
      turnSystem_on(MAX_ON_TIME);
    }
  }
  else
  {
    if (systemState == 1)
    {
      turnSystem_off();
    }
  }
}
void setup()
{
  Serial.begin(9600);
  Serial.println("Begin!");
  initHW();
  tempSensor_init();

  check_EEPROMVal();
  read_EEPROMval(0);
  read_EEPROMval(1);

  startLEDdisplay();
  LCDdisplay.CenterTXT("Aquarium V1.0 9/20", "BootUp");
  delay(2000);
  // BEEP
}

void loop()
{
  read_butLoop();
  check_temp_loop();
  displayLED();

  delay(50);
}
