/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef Morse_h
#define Morse_h



#include "Arduino.h"


class Morse
{
  public:
    Morse(int pin);
    void dot();
    void dash();
    bool run();
  private:
    // int _pin;
};

#endif
