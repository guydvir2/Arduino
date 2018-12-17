#ifndef Morse_h
#define Morse_h

// #include "Arduino.h"

class Morse
{
  public:
    Morse(const char *ssid, const char *pwd);
    void dot();
    void dash();
    void startNetwork();
  private:
    int _pin;
    const char _SSID;
    const char _PWD;

};

#endif
