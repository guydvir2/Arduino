#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
const int PIN_TO_SENSOR_1 = 2;
const int PIN_TO_SENSOR_2 = 3;
const int PWR_SENSOR_1 = 6;
const int PWR_SENSOR_2 = 4;
const int BUZ_PIN = 5;
const int DET_DURATION = 5; // sec in detection

bool detection = false;
int detCounter = 0;
char lines[2][20];

void make_buz(byte i, byte del = 50)
{
  for (int x = 0; x < (int)i; x++)
  {
    digitalWrite(BUZ_PIN, HIGH);
    delay(del);
    digitalWrite(BUZ_PIN, LOW);
    delay(del);
    x++;
  }
}
void quick_buzz()
{
  make_buz(1, 5);
}

class PIRsensor
{
  typedef void (*cb_func)();

private:
  int _pin;
  int _length_logic_state = 5;
  bool _use_detfunc = false;
  bool _use_enddetfunc = false;
  bool _lastState = false;
  bool _isDetect = true;
  bool _timer_is_on = false;
  long _lastDetection_clock = 0;
  long _endTimer = 0;

  cb_func _run_func;
  cb_func _run_enddet_func;

public:
  int detCounts = 0;
  int ignore_det_interval = 20;
  int timer_duration = 10;
  int delay_first_detection = 15; // seconds
  char *sensNick = "sensor";
  bool use_timer = false;
  bool sens_state = false;
  bool logic_state = false; // a flag that will be on for some time, altghot physucal state has changed

  PIRsensor(int Pin, char *nick = "PIRsensor", int logic_length = 5)
  {
    _pin = Pin;
    sensNick = nick;
    _length_logic_state = logic_length;
  }
  void start()
  {
    pinMode(_pin, INPUT);
    digitalWrite(_pin, !_isDetect);
  }
  void run_func(cb_func cb)
  {
    _run_func = cb;
    _use_detfunc = true;
  }
  void run_enddet_func(cb_func cb)
  {
    _run_enddet_func = cb;
    _use_enddetfunc = true;
  }

  bool checkSensor()
  {
    sens_state = digitalRead(_pin); // physical sensor read
    // if (digitalRead(_pin)!=sens_state){
    //   sens_state = !sens_state;
    //   Serial.print(sensNick);
    //   Serial.print(" state: ");
    //   Serial.println(sens_state);
    // }
      
    bool ignore_det = millis() > _lastDetection_clock + (long)ignore_det_interval * 1000 + (long)(_length_logic_state * 1000); // timeout - minimal time between detections
    bool first_det = millis() > (long)delay_first_detection * 1000;                                                            // timeout - detection after boot
    logic_state = (millis() <= (_lastDetection_clock + (long)(_length_logic_state * 1000)));                                   // logic flag for sensor to be indetection mode
    bool end_to = use_timer && _timer_is_on && (millis() > _endTimer);                                                         // timeout for

    if (sens_state == _isDetect && _lastState == false && logic_state == false)
    {
      if (ignore_det || first_det)
      {
        Serial.print("~~~~~~~~~~~~~detect_");
        Serial.println(sensNick);
        _lastState = true;
        detCounts++;

        _lastDetection_clock = millis();
        if (_use_detfunc)
        {
          _run_func();
        }
        if (use_timer)
        {
          _endTimer = millis() + (long)timer_duration * 1000;
          _timer_is_on = true;
        }
        return 1;
      }
    }
    else if (sens_state != _isDetect && _lastState && logic_state == false)
    {
      _lastState = false;
      _lastDetection_clock = 0;
      Serial.print("~~~~~~~~~~~~~~~~~~~END_");
      Serial.println(sensNick);
      return 0;
    }
    else
    {
      return 0;
    }
  }
};

PIRsensor sensor0(PIN_TO_SENSOR_1, "Sensor_1", 10);
PIRsensor sensor1(PIN_TO_SENSOR_2, "Sensor_2",10);

void setup()
{
  Serial.begin(9600);
  Serial.println("START!");
  sensor0.use_timer = false;
  sensor0.timer_duration = 15;
  sensor0.ignore_det_interval = 5;
  // sensor0.run_func(quick_buzz);
  sensor0.start();

  sensor1.use_timer = false;
  sensor1.timer_duration = DET_DURATION;
  sensor1.ignore_det_interval = 5;
  // sensor1.run_func(quick_buzz);
  sensor1.start();

  pinMode(PWR_SENSOR_1, OUTPUT);
  pinMode(PWR_SENSOR_2, OUTPUT);
  pinMode(BUZ_PIN, OUTPUT);
  digitalWrite(PWR_SENSOR_1, 1);
  digitalWrite(PWR_SENSOR_2, 1);

  lcd.init(); // initialize the lcd
  lcd.backlight();
}

void loop()
{
  char det_word[20];
  sensor0.checkSensor();
  sensor1.checkSensor();
  bool s1 = sensor1.logic_state;
  bool s0 = sensor0.logic_state;
  sprintf(det_word, "[%s][%s][%s]", s0 ? "ON " : "OFF", s1 ? "ON " : "OFF", s0 && s1 ? "DET " : "FREE");

  if (s0 && s1 && detection == false)
  {
    make_buz(2, 20);
    detection = true;
    detCounter++;
  }
  else if (s0 == false && s1 == false && detection == true)
  {
    detection = false;
  }
  sprintf(lines[0], "%s", det_word);
  sprintf(lines[1], "[#%d][#%d][#%d]", detCounter, sensor0.detCounts, sensor1.detCounts);
  lcd.setCursor(0, 0);
  lcd.print(det_word);
  lcd.setCursor(0, 1);
  lcd.print(lines[1]);
  delay(500);
}
