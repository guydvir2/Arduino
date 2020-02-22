#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
const int PIN_TO_SENSOR_1 = 2;
const int PIN_TO_SENSOR_2 = 3;
const int PWR_SENSOR_2 = 4;
bool detection = false;
int detCounter = 0;
int false_sens_1 = 0;
int false_sens_2 = 0;
char tempstr[20];

long timeout_next_det = 0;

void printlin1(char *txt)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(txt);
}
void printlin2(int i)
{
  lcd.setCursor(0, 1);
  char t[20];
  sprintf(t, "detections:#%d", i);
  lcd.print(t);
}

void setup()
{
  Serial.begin(9600);
  pinMode(PIN_TO_SENSOR_1, INPUT); // set arduino pin to input mode to read value from OUTPUT pin of sensor
  pinMode(PIN_TO_SENSOR_2, INPUT); // set arduino pin to input mode to read value from OUTPUT pin of sensor
  pinMode(PWR_SENSOR_2, OUTPUT);

  digitalWrite(PIN_TO_SENSOR_1, 0);
  digitalWrite(PIN_TO_SENSOR_2, 0);
  digitalWrite(PWR_SENSOR_2, 1);

  lcd.init(); // initialize the lcd
  lcd.backlight();
}

void loop()
{
  if (digitalRead(PIN_TO_SENSOR_2) && digitalRead(PIN_TO_SENSOR_1) && detection == false )
  {
    detCounter++;
    Serial.print("Detections: ");
    Serial.println(detCounter);
    timeout_next_det = millis();
    detection = true;
  }
  else if ((digitalRead(PIN_TO_SENSOR_2) && digitalRead(PIN_TO_SENSOR_1) == 0 && detection == false) || (digitalRead(PIN_TO_SENSOR_2) == 0 && digitalRead(PIN_TO_SENSOR_1) && detection == false))
  {
    // Serial.print("Partial detection: [");
    if (digitalRead(PIN_TO_SENSOR_1))
    {
      false_sens_1++;
    }
    else if (digitalRead(PIN_TO_SENSOR_2))
    {
      false_sens_2++;
    }
  }
  else if (digitalRead(PIN_TO_SENSOR_2) == 0 && digitalRead(PIN_TO_SENSOR_1) == 0 && detection == true)
  {
    detection = false;
    Serial.println("Detection ended");
  }
  if (detection)
  {
    printlin1("Detection");
  }
  else
  {
    printlin1("NO-Detection");
  }
  printlin2(detCounter);
  // sprintf(tempstr, "[%d,%d,%d]", detCounter, false_sens_1, false_sens_2);
  // printlin2(tempstr);
  delay(200);
}
