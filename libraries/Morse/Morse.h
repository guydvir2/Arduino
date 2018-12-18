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
<<<<<<< HEAD
    // int _pin;
=======
    int _pin;
    const char _SSID;
    const char _PWD;

>>>>>>> d503db23c0f8626d760da71ff1ac5091d2968880
};

#endif
