#include "Arduino.h"
#include "myTimeoutSwitch.h"

timeOUTSwitch::timeOUTSwitch(bool saveCLK) : CLKstore("ClkStore.json", true)
{
    _useSavedCLK = saveCLK;
}
void timeOUTSwitch::def_funcs(func_cb startF, func_cb endF)
{
    _startf = startF; /*External function to be executed when triggered */
    _endf = endF;     /*External function to be executed when triggered */

    if (_useSavedCLK)
    {
        CLKstore.start();
        _chk_rem_after_boot();
    }
}
void timeOUTSwitch::start_TO(int _TO, char *src)
{
    TO_duration_sec = _TO;
    TO_start_millis = millis();
    if (inTO == false)
    {
        _startf(src);
    }
    else{
        Serial.println("timeupdate");
    }
    inTO = true;
    _updateEndClk(TO_duration_sec, now());
}
void timeOUTSwitch::finish_TO(char *src)
{
    _endf(src); /*calling first to get remTime correct on MQTT msg */
    clearTO();
}
void timeOUTSwitch::restart_TO(int _TO)
{
    // if (_TO == 0)
    // {
    //     start_TO(TO_duration_sec, "Restart");
    // }
    // else
    // {
    //     start_TO(_TO, "zcz");
    // }
}
void timeOUTSwitch::startIO(int _in_IO, bool _instate)
{
    _IN_io = _in_IO;
    _inputstatOn = _instate;
    useInput = true;

    if (_inputstatOn == LOW)
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
        CLKstore.setValue(_keyJSON, 0);
    }
}
int timeOUTSwitch::remTime()
{
    if (inTO)
    {
        return (int)(TO_endclk - now());
    }
    else
    {
        return 0;
    }
}

void timeOUTSwitch::_TOlooper()
{
    if (TO_start_millis != 0)
    {
        if (millis() - TO_start_millis > +TO_duration_sec * 1000UL || millis() - TO_start_millis > maxON_minutes * 1000UL)
        {
            finish_TO("timeOut");
        }
    }
}
void timeOUTSwitch::_input_looper()
{
    if (useInput)
    {
        bool currentRead_0 = digitalRead(_IN_io);
        delay(50);
        bool currentRead_1 = digitalRead(_IN_io);

        if (currentRead_0 == currentRead_1 && currentRead_0 != _lastinput)
        {
            if (trigType == 0 && currentRead_0 == _inputstatOn) /*button is pressed ON */
            {
                if (inTO == true)
                {
                    finish_TO("Button");
                }
                else
                {
                    start_TO(def_TO_minutes, "Button");
                }
            }
            else if (trigType == 1) /* Case Switch*/
            {
                if (currentRead_0 == _inputstatOn)
                {
                    start_TO(def_TO_minutes, "Button");
                }
                else
                {
                    finish_TO("Button");
                }
            }
            else if (trigType == 2 && currentRead_0 == _inputstatOn) /* Case of sensor that each detection resets its timeout */
            {
                start_TO(def_TO_minutes, "Sensor");
            }
            _lastinput = currentRead_0;
        }
    }
}
void timeOUTSwitch::_updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk)
{
    if (TO_start_clk == 0)
    {
        TO_start_clk = now();
    }
    TO_endclk = TO_start_clk + TO_dur_minutes;
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyJSON, (long)TO_endclk);
    }
}
void timeOUTSwitch::_chk_rem_after_boot()
{
    long bb = 0;
    CLKstore.getValue(_keyJSON, bb);

    if (bb > 0 && bb - now() > 0)
    {
        start_TO(bb - now(), "Resume");
    }
}
