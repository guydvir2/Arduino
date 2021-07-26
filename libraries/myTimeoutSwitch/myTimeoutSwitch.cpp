#include "Arduino.h"
#include "myTimeoutSwitch.h"

timeOUTSwitch::timeOUTSwitch(bool saveCLK) : CLKstore("/ClkStore.json")
{
    _useSavedCLK = saveCLK;
    // _counter++;
}
void timeOUTSwitch::def_funcs(func_cb startF, func_cb endF)
{
    _startf = startF; /*External function to be executed when triggered */
    _endf = endF;     /*External function to be executed when triggered */

    if (_useSavedCLK)
    {
        // Create desticntive name in case of several instances
        sprintf(_keyEnd, "endClk_%d", icount);
        sprintf(_keyStart, "strtClk_%d", icount);
        sprintf(_keyCounter, "Count_%d", icount);
        CLKstore.start();
        _chk_rem_after_boot();
    }
}
void timeOUTSwitch::start_TO(int _TO, byte src, bool minutes)
{
    TO_duration = _TO; // given in seconds
    if (minutes)
    {
        TO_duration *= 60; // in case given in minutes
    }
    TO_start_millis = millis();
    // if (inTO == false)
    // {
    _startf(src, icount);
    if (onClk() == 0)
    {
        _updateStartClk(now());
        _updateEndClk(TO_duration, now());
    }
    if (trigType == 2)
    {
        _updateEndClk(TO_duration, now());
    }
    // }
    // else if (trigType == 0)
    // {
    //     _startf(src, icount);
    // }
    // if (inTO == false && onClk() == 0)
    // {
    //     _updateStartClk(now());
    // }
    inTO = true;
}
void timeOUTSwitch::finish_TO(byte src)
{
    _endf(src, icount); /*calling first to get remTime correct on MQTT msg */
    clearTO();
}
void timeOUTSwitch::startIO(int _in_IO, bool _instate, bool _reverseInput)
{
    _IN_io = _in_IO;
    _inputstatOn = _instate;

    if (_inputstatOn == LOW)
    {
        pinMode(_IN_io, INPUT_PULLUP);
    }
    else if (_reverseInput)
    {
        pinMode(_IN_io, INPUT_PULLUP);
    }
    else
    {
        pinMode(_IN_io, INPUT);
    }
    _lastinput = digitalRead(_IN_io);
}
void timeOUTSwitch::looper()
{
    _input_looper();
    _TOlooper();
}
void timeOUTSwitch::clearTO()
{
    inTO = false;
    TO_start_millis = 0;
    TO_endclk = 0;
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyEnd, 0);
    }
    _updateStartClk(0);
}
int timeOUTSwitch::remTime()
{
    long bb = 0;
    CLKstore.getValue(_keyEnd, bb);
    if (bb > 0 && bb - now() > 0)
    {
        return bb - now();
    }
    else
    {
        return 0;
    }
}
time_t timeOUTSwitch::onClk()
{
    long bb = 0;
    CLKstore.getValue(_keyStart, bb);
    return bb;
}
byte timeOUTSwitch::getCount()
{
    int a = 0;
    CLKstore.getValue(_keyCounter, a);
    return (byte)a;
}

void timeOUTSwitch::_TOlooper()
{
    if (TO_start_millis != 0)
    {
        if (millis() - TO_start_millis > TO_duration * 1000UL || millis() - TO_start_millis > maxON_minutes * 60000UL)
        {
            finish_TO(1);
        }
    }
}
void timeOUTSwitch::_input_looper()
{
    byte press_to_off = 2; //seconds. after this re-press PWM will set OFF
    if (useInput)
    {
        bool currentRead_0 = digitalRead(_IN_io);
        delay(50);
        bool currentRead_1 = digitalRead(_IN_io);

        if (currentRead_0 == currentRead_1 && currentRead_0 != _lastinput)
        {
            if (trigType == 0 && currentRead_0 == _inputstatOn) /* momnetary button is pressed */
            {
                bool cond_a = (pCounter == 0) || (pCounter < max_pCount && millis() - _lastPress < 1000 * press_to_off); /* first press on, or inc intensity */
                bool cond_b = (pCounter >= max_pCount) || (millis() - _lastPress > 1000 * press_to_off);                 /* when press is far from last - turn off */

                if (cond_a || !inTO)
                {
                    pCounter++;
                    start_TO(def_TO_minutes, 0);
                    _lastPress = millis();
                }
                else if (inTO == true || cond_b)
                {
                    finish_TO(0);
                    pCounter = 0;
                    _lastPress = 0;
                }
                else
                {
                    Serial.println("State not defined");
                }
            }
            else if (trigType == 1) /* Switch is set ON or OFF */
            {
                if (currentRead_0 == _inputstatOn)
                {
                    start_TO(def_TO_minutes, 0);
                }
                else
                {
                    finish_TO(0);
                }
            }
            else if (trigType == 2 && currentRead_0 == _inputstatOn) /* Case of sensor that each detection restarts its timeout */
            {
                start_TO(def_TO_minutes, 0);
            }
            _lastinput = currentRead_0;
        }
    }
}
void timeOUTSwitch::_updateEndClk(int _TO_dur, unsigned long TO_start_clk)
{
    if (TO_start_clk == 0)
    {
        TO_start_clk = now();
    }
    TO_endclk = TO_start_clk + _TO_dur;
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyEnd, (long)TO_endclk);
        CLKstore.setValue(_keyCounter, pCounter);
    }
}
void timeOUTSwitch::_updateStartClk(long TO_start_clk)
{
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyStart, TO_start_clk);
    }
}
void timeOUTSwitch::_chk_rem_after_boot()
{
    long bb = 0;
    bool record = CLKstore.getValue(_keyEnd, bb);

    if (record)
    {
        if (bb > 0 && bb - now() > 0)
        {
            start_TO(bb - now(), 1, false);
        }
        else if (bb > 0 && bb - now() < 0)
        {
            clearTO();
        }
    }
    else
    {
        clearTO();
    }
}
// byte timeOUTSwitch::_counter = 0;
