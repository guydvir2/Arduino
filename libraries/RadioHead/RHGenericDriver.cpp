// RHGenericDriver.cpp
//
// Copyright (C) 2014 Mike McCauley
<<<<<<< HEAD
// $Id: RHGenericDriver.cpp,v 1.18 2015/01/02 21:38:24 mikem Exp $
=======
// $Id: RHGenericDriver.cpp,v 1.23 2018/02/11 23:57:18 mikem Exp $
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787

#include <RHGenericDriver.h>

RHGenericDriver::RHGenericDriver()
    :
    _mode(RHModeInitialising),
    _thisAddress(RH_BROADCAST_ADDRESS),
    _txHeaderTo(RH_BROADCAST_ADDRESS),
    _txHeaderFrom(RH_BROADCAST_ADDRESS),
    _txHeaderId(0),
    _txHeaderFlags(0),
    _rxBad(0),
    _rxGood(0),
<<<<<<< HEAD
    _txGood(0)
=======
    _txGood(0),
    _cad_timeout(0)
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
{
}

bool RHGenericDriver::init()
{
    return true;
}

// Blocks until a valid message is received
void RHGenericDriver::waitAvailable()
{
    while (!available())
	YIELD;
}

// Blocks until a valid message is received or timeout expires
// Return true if there is a message available
// Works correctly even on millis() rollover
bool RHGenericDriver::waitAvailableTimeout(uint16_t timeout)
{
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
        if (available())
<<<<<<< HEAD
           return true;
=======
	{
           return true;
	}
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
	YIELD;
    }
    return false;
}

bool RHGenericDriver::waitPacketSent()
{
    while (_mode == RHModeTx)
	YIELD; // Wait for any previous transmit to finish
    return true;
}

bool RHGenericDriver::waitPacketSent(uint16_t timeout)
{
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
        if (_mode != RHModeTx) // Any previous transmit finished?
           return true;
	YIELD;
    }
    return false;
}

<<<<<<< HEAD
=======
// Wait until no channel activity detected or timeout
bool RHGenericDriver::waitCAD()
{
    if (!_cad_timeout)
	return true;

    // Wait for any channel activity to finish or timeout
    // Sophisticated DCF function...
    // DCF : BackoffTime = random() x aSlotTime
    // 100 - 1000 ms
    // 10 sec timeout
    unsigned long t = millis();
    while (isChannelActive())
    {
         if (millis() - t > _cad_timeout) 
	     return false;
#if (RH_PLATFORM == RH_PLATFORM_STM32) // stdlib on STMF103 gets confused if random is redefined
	 delay(_random(1, 10) * 100);
#else
         delay(random(1, 10) * 100); // Should these values be configurable? Macros?
#endif
    }

    return true;
}

// subclasses are expected to override if CAD is available for that radio
bool RHGenericDriver::isChannelActive()
{
    return false;
}

>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
void RHGenericDriver::setPromiscuous(bool promiscuous)
{
    _promiscuous = promiscuous;
}

void RHGenericDriver::setThisAddress(uint8_t address)
{
    _thisAddress = address;
}

void RHGenericDriver::setHeaderTo(uint8_t to)
{
    _txHeaderTo = to;
}

void RHGenericDriver::setHeaderFrom(uint8_t from)
{
    _txHeaderFrom = from;
}

void RHGenericDriver::setHeaderId(uint8_t id)
{
    _txHeaderId = id;
}

void RHGenericDriver::setHeaderFlags(uint8_t set, uint8_t clear)
{
    _txHeaderFlags &= ~clear;
    _txHeaderFlags |= set;
}

uint8_t RHGenericDriver::headerTo()
{
    return _rxHeaderTo;
}

uint8_t RHGenericDriver::headerFrom()
{
    return _rxHeaderFrom;
}

uint8_t RHGenericDriver::headerId()
{
    return _rxHeaderId;
}

uint8_t RHGenericDriver::headerFlags()
{
    return _rxHeaderFlags;
}

<<<<<<< HEAD
int8_t RHGenericDriver::lastRssi()
=======
int16_t RHGenericDriver::lastRssi()
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
{
    return _lastRssi;
}

RHGenericDriver::RHMode  RHGenericDriver::mode()
{
    return _mode;
}

void  RHGenericDriver::setMode(RHMode mode)
{
    _mode = mode;
}

bool  RHGenericDriver::sleep()
{
    return false;
}

// Diagnostic help
void RHGenericDriver::printBuffer(const char* prompt, const uint8_t* buf, uint8_t len)
{
<<<<<<< HEAD
    uint8_t i;

#ifdef RH_HAVE_SERIAL
    Serial.println(prompt);
=======
#ifdef RH_HAVE_SERIAL
    Serial.println(prompt);
    uint8_t i;
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
    for (i = 0; i < len; i++)
    {
	if (i % 16 == 15)
	    Serial.println(buf[i], HEX);
	else
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(' ');
	}
    }
    Serial.println("");
#endif
}

uint16_t RHGenericDriver::rxBad()
{
    return _rxBad;
}

uint16_t RHGenericDriver::rxGood()
{
    return _rxGood;
}

uint16_t RHGenericDriver::txGood()
{
    return _txGood;
}

<<<<<<< HEAD
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined(RH_PLATFORM_ATTINY)
=======
void RHGenericDriver::setCADTimeout(unsigned long cad_timeout)
{
    _cad_timeout = cad_timeout;
}

#if (RH_PLATFORM == RH_PLATFORM_ATTINY)
>>>>>>> d27e11fba5c87a25cf468b826ee28f6e60831787
// Tinycore does not have __cxa_pure_virtual, so without this we
// get linking complaints from the default code generated for pure virtual functions
extern "C" void __cxa_pure_virtual()
{
    while (1);
}
#endif
