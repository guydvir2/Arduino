#include <Arduino.h>
#include <buttonPresses.h>

#define RELAY_ON HIGH
struct MSGstr
{
  bool state;     /* Up/Down/ Off */
  uint8_t reason; /* What triggered the button */
};

enum REASONS : const uint8_t
{
  TIMEOUT,
  BUTTON,
  BUTTON2,
  LCKDOWN,
};
enum WIN_STATES : const uint8_t
{
  STOP,
  UP,
  DOWN,
  ERR
};

class Window
{
private:
  buttonPresses _windowSwitch;
  buttonPresses _windowSwitch_ext;

  bool _lockdownState = false;
  bool _uselockdown = false;
  bool _useTimeout = false;
  bool _useExtSW = false;

  uint8_t _outpins[2];

  int _timeout_clk = 0; // seconds to release relay
  unsigned long _timeoutcounter = 0;

public:
  bool newMSGflag = false;
  MSGstr MSG;

public:
  Window()
  {
  }
  void def(uint8_t upin, uint8_t dpin, uint8_t outup_pin, uint8_t outdown_pin)
  {
    _outpins[0] = outup_pin;
    _outpins[1] = outdown_pin;

    _windowSwitch.pin0 = upin;
    _windowSwitch.pin1 = dpin;
    _windowSwitch.buttonType = 2;
  }
  void def_extSW(uint8_t upin, uint8_t dpin)
  {
    _useExtSW = true;
    _windowSwitch_ext.pin0 = upin;
    _windowSwitch_ext.pin1 = dpin;
    _windowSwitch_ext.buttonType = 2;
  }
  void def_extras(bool useTimeout = true, bool useLockdown = true, int timeout_clk = 15)
  {
    _useTimeout = useTimeout;
    _uselockdown = useLockdown;
    _timeout_clk = timeout_clk;
  }
  void start()
  {
    _windowSwitch.start();
    if (_useExtSW)
    {
      _windowSwitch_ext.start();
    }

    pinMode(_outpins[0], OUTPUT);
    pinMode(_outpins[1], OUTPUT);
    _allOff();
  }
  void init_lockdown()
  {
    if (_uselockdown && _lockdownState == false)
    {
      _switch_cb(DOWN, LCKDOWN);
      _lockdownState = true;
    }
  }
  void release_lockdown()
  {
    if (_uselockdown && _lockdownState == true)
    {
      _lockdownState = false;
    }
  }
  void loop()
  {
    _readSW();
    _timeout_looper();
  }
  void ext_SW(uint8_t state, uint8_t reason)
  {
    _switch_cb(state, reason);
  }
  uint8_t get_winState()
  {
    bool uprel = digitalRead(_outpins[0]);
    bool downrel = digitalRead(_outpins[1]);

    if (downrel == !RELAY_ON && uprel == !RELAY_ON)
    {
      return STOP;
    }
    else if (downrel == RELAY_ON && uprel == !RELAY_ON)
    {
      return DOWN;
    }
    else if (downrel == !RELAY_ON && uprel == RELAY_ON)
    {
      return UP;
    }
    else
    {
      return ERR;
    }
  }

private:
  void _allOff()
  {
    digitalWrite(_outpins[0], !RELAY_ON);
    digitalWrite(_outpins[1], !RELAY_ON);
    delay(10);
  }
  void _winUP()
  {
    _allOff();
    digitalWrite(_outpins[0], RELAY_ON);
  }
  void _winDOWN()
  {
    _allOff();
    digitalWrite(_outpins[1], RELAY_ON);
  }
  void _switch_cb(uint8_t state, uint8_t i)
  {
    if ((_uselockdown && _lockdownState == false || _uselockdown == false)&& state != get_winState())
    {
      if (state == STOP)
      {
        _allOff();
        newMSGflag = true;
        _timeoutcounter = 0;
        MSG.state = state;
        MSG.reason = i;
      }
      else if (state == UP)
      {
        _winUP();
        newMSGflag = true;
        _timeoutcounter = millis();
        MSG.state = state;
        MSG.reason = i;
      }
      else if (state == DOWN)
      {
        _winDOWN();
        newMSGflag = true;
        _timeoutcounter = millis();
        MSG.state = state;
        MSG.reason = i;
      }
      else
      {
        return;
      }
    }
  }
  void _readSW()
  {
    uint8_t switchRead = _windowSwitch.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
    if (switchRead < 3)
    {
      _switch_cb(switchRead, BUTTON);
      return;
    }
    if (_useExtSW)
    {
      switchRead = _windowSwitch_ext.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
      if (switchRead < 3)
      {
        _switch_cb(switchRead, BUTTON2);
        return;
      }
    }
  }
  void _timeout_looper()
  {
    if (_useTimeout && _timeoutcounter > 0)
    {
      if (millis() - _timeoutcounter > _timeout_clk*1000)
      {
        _switch_cb(STOP, TIMEOUT);
      }
    }
  }
};

Window WIN_SW;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nSTART");

  WIN_SW.def(D5, D6, D3, D4);
  WIN_SW.def_extSW(D7,D2);
  WIN_SW.def_extras();
  WIN_SW.start();
}

void loop()
{
  // put your main code here, to run repeatedly:
  WIN_SW.loop();
  if (WIN_SW.newMSGflag)
  {
    Serial.print("State: ");
    Serial.print(WIN_SW.MSG.state);
    Serial.print("\tReason: ");
    Serial.println(WIN_SW.MSG.reason);
    WIN_SW.newMSGflag = false;
  }
}
