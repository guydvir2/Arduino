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
// ~~ LCD Display ~~
myLCD LCDdisplay(16, 2);

#define MAX_T 32.0
#define MIN_T 20.0
#define INC_T 0.5
#define START_T 28.0
#define STOP_T 30.0
#define MAX_ON_TIME 60 * 12      //max minutes ON
#define TIMEOUT_AFTER_ON 60      // minutes timeout after OFF between re-ON
#define MANUAL_ON_TIMEOUT 5 // manual ON minutes

int temp_ptr = 0;
int systemState = 0; // 0:Off, 1:on 2:SETUP
float temp_values[2] = {START_T, STOP_T};
float tempC = 0;
bool manualON = false;
bool masterArnON = false;

// ~~~~~~ GPIO settings ~~~~~~~
#define pressed LOW
#define LED_ON HIGH
#define relayON HIGH

const int button1Pin = 2;
const int button2Pin = 3;
const int but0LEDPin = 5;
const int but1LEDPin = 6;
const int relayPin = 7;
const int masterArmPin = 8;

const int buzzerPin = 12;
const int *butPins[] = {&button1Pin, &button2Pin};
const int *ledPins[2] = {&but0LEDPin, &but1LEDPin};

float press_time = 0.0;
const float longPress = 1.0;
const float timeout_longPress = 5.0;
unsigned long lastPress_clock = 0;
unsigned long start_run_clock = 0;
unsigned long end_run_clock = 0;
unsigned long last_end_clock = 0;

// ~~~~~~~ System States ~~~~~~
void turnSystem_on(int timeout, char *msg = "")
{
  if (millis() > last_end_clock + 2)
  {
    systemState = 1;
    start_run_clock = millis();
    end_run_clock = start_run_clock + 1000 * timeout * 60L;
    LCDdisplay.clear();

    if (strcmp(msg, "") == 0)
    {
      LCDdisplay.CenterTXT("System Operation", "ON");
    }
    else
    {
      LCDdisplay.CenterTXT("System Operation", msg);
    }

    digitalWrite(relayPin, HIGH);
    delay(2000);
    // Buzz ON
  }
}
void turnSystem_off(char *msg = "")
{
  systemState = 0;
  start_run_clock = 0;
  end_run_clock = 0;
  manualON = false;
  last_end_clock = millis();
  LCDdisplay.clear();
  if (strcmp(msg, "") == 0)
  {
    LCDdisplay.CenterTXT("System Operation", "OFF");
  }
  else
  {
    LCDdisplay.CenterTXT("System Operation", msg);
  }
  digitalWrite(relayPin, LOW);

  delay(2000);
  //Buzz OFF
}
void start_manualOP()
{
  Serial.println("Start_manual_OP");
  turnSystem_on(MANUAL_ON_TIMEOUT, "Manual Start");
  manualON = true;
}
void manualOFF()
{
  Serial.println("Manual OFF");
  turnSystem_off("Manual OFF");
}
void start_setupMode()
{
  Serial.println("Enter SetUpMode");
  Serial.print("Start Temp: ");
  Serial.println(temp_values[0]);
  Serial.print("Stop Temp: ");
  Serial.println(temp_values[1]);

  temp_ptr = 0;
  systemState = 2;
}
void end_setupMode()
{
  Serial.println("Exit SetUpMode");
  systemState = 0;
  temp_ptr = 0;
}
void timeout_check()
{
  if (systemState == 1)
  {
    if (millis() > end_run_clock)
    {
      Serial.println("Reached Timeout");
      turnSystem_off();
    }
  }
}
void check_masterArm()
{
  masterArnON = digitalRead(masterArmPin);
}
// ~~~ Button Manipulations ~~~
void inc_value()
{
  Serial.println("increase value");
  if (temp_values[temp_ptr] + INC_T <= MAX_T)
  {
    temp_values[temp_ptr] += INC_T;
  }
  else
  {
    temp_values[temp_ptr] = MIN_T;
  }
  Serial.print("temp store #");
  Serial.print(temp_ptr);
  Serial.print(": ");
  Serial.println(temp_values[temp_ptr]);
}
void dec_value()
{
  Serial.println("dec value");
  if (temp_values[temp_ptr] - INC_T >= MIN_T)
  {
    temp_values[temp_ptr] -= INC_T;
  }
  else
  {
    temp_values[temp_ptr] = MIN_T;
  }
  Serial.print("temp store #");
  Serial.print(temp_ptr);
  Serial.print(": ");
  Serial.println(temp_values[temp_ptr]);
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
        manualOFF();
      }
      else if (press_time > longPress && systemState == 0)
      {
        start_setupMode();
      }
      else if (press_time > longPress && systemState == 2)
      {
        writeEEPROMval(temp_ptr, temp_values[temp_ptr]);
        if (temp_ptr == 0)
        {
          temp_ptr++;
        }
        else if (temp_ptr == 1)
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
  for (int a = 0; a < 2; a++)
  {
    readButton(*butPins[a]);
  }
}

// ~~~~~~ Display ~~~~~~
void create_clock(unsigned long int_time, char retClock[20])
{
  byte days = 0;
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;

  int sec2minutes = 60;
  int sec2hours = (sec2minutes * 60);
  int sec2days = (sec2hours * 24);
  int sec2years = (sec2days * 365);

  days = (int)(int_time / sec2days);
  hours = (int)((int_time - days * sec2days) / sec2hours);
  minutes = (int)((int_time - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(int_time - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  sprintf(retClock, "%01dd %02d:%02d:%02d", days, hours, minutes, seconds);
}
void startLEDdisplay()
{
  LCDdisplay.start();
}
void LCDdisaply_loop()
{
  char a[10];
  char b[20];
  char t[10];
  char line0[20];
  char line1[20];
  LCDdisplay.clear();
  dtostrf(tempC, 4, 1, t);
  if (!masterArnON)
  {
    sprintf(line0, "#SYSTEM DISABLED#");
    sprintf(line1, "T:%sC", t);
    LCDdisplay.CenterTXT(line0, line1);
    delay(250);
  }
  else
  {
    if (systemState == 0)
    {
      dtostrf(temp_values[0], 4, 1, a);
      dtostrf(temp_values[1], 4, 1, b);
      sprintf(line0, "#OFF# T:%sC", t);
      sprintf(line1, "H:%s L:%s", a, b);
    }
    else if (systemState == 1)
    {
      create_clock((int)((millis() - start_run_clock) / 1000), b);
      sprintf(line0, "#ON# T:%sC", t);
      sprintf(line1, "%s", b);
    }
    else if (systemState == 2)
    {
      dtostrf(temp_values[0], 4, 1, a);
      dtostrf(temp_values[1], 4, 1, b);
      sprintf(line0, "#SETUP MODE#");
      if (temp_ptr == 0)
      {
        sprintf(line1, "H:%sC         ", a);
      }
      else if (temp_ptr == 1)
      {
        sprintf(line1, "         L:%sC", b);
      }
    }
    LCDdisplay.CenterTXT(line0, line1);
    if (systemState == 2)
    {
      delay(50);
    }
    else
    {
      delay(250);
    }
  }
}

// ~~~~~~ EEPROM ~~~~~~
void check_EEPROMVal()
{
  int arr[4] = {(int)temp_values[0], 0, (int)temp_values[1], 0};
  for (int a = 0; a < 4; a++)
  {
    if (EEPROM.read(a) == 255)
    {
      EEPROM.write(a, arr[a]);
      Serial.println(EEPROM.read(a));
    }
  }
  temp_values[0] = (float)EEPROM.read(0) + (float)EEPROM.read(1) / 10.0;
  temp_values[1] = (float)EEPROM.read(2) + (float)EEPROM.read(3) / 10.0;
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
  for (int a = 0; a < 2; a++)
  {
    pinMode(*butPins[a], INPUT_PULLUP);
    pinMode(*ledPins[a], OUTPUT);
  }
  pinMode(masterArmPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);
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
  int read_interval = 30; //seconds
  if (millis() - lastRead > read_interval * 1000 || tempC == 0)
  {
    TempSensor.requestTemperatures();
    tempC = TempSensor.getTempC(tempSensorAdress);
    // Serial.print("Temp C: ");
    // Serial.println(tempC);
    lastRead = millis();
  }
}
void check_temp_loop()
{
  getTemperature();
  if (manualON == false)
  {
    if (tempC >= temp_values[0] && tempC >= temp_values[1])
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
}

void buttonLEDs_loop()
{
  static unsigned long lastBlink = 0;
  const int blinktime = 1000;

  if (systemState == 0)
  {
    digitalWrite(*ledPins[0], LED_ON);
    digitalWrite(*ledPins[1], LED_ON);
  }
  else if (systemState == 1)
  {
    if (millis >= lastBlink + blinktime)
    {
      digitalWrite(*ledPins[0], !digitalRead(*ledPins[0]));
      lastBlink = millis();
    }
  }
  else if (systemState == 2)
  {
    if (millis >= lastBlink + blinktime)
    {
      digitalWrite(*ledPins[0], !digitalRead(*ledPins[0]));
      digitalWrite(*ledPins[1], !digitalRead(*ledPins[1]));
      lastBlink = millis();
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
  LCDdisplay.CenterTXT("Aquarium Cooler", "V09/2020");
  delay(2000);
  // BEEP
}

void loop()
{
  if (masterArnON)
  {
    read_butLoop();
  }
  if (systemState != 2)
  {
    check_temp_loop();
    if (masterArnON)
    {
      buttonLEDs_loop();
      timeout_check();
    }
  }
  if (!masterArnON)
  {
    digitalWrite(relayPin, !relayON);
  }
  check_masterArm();
  LCDdisaply_loop();

  delay(50);
}
