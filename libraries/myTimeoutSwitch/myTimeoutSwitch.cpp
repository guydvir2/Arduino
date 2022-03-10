#include <myTimeoutSwitch.h>

timeOUTSwitch::timeOUTSwitch(bool saveCLK) : CLKstore("/ClkStore.json", true, 100)
{
    _useSavedCLK = saveCLK;
    // _counter++;
}
void timeOUTSwitch::startIO(int _in_IO, bool _instate)
{
    _IN_io = _in_IO;
    _inputstatOn = _instate;
    useInput = true;

    if (_inputstatOn == LOW)
    {
        pinMode(_IN_io, INPUT_PULLUP); /* Input that trigers LOW, shuch as buttons that have pullups*/
    }
    else
    {
        pinMode(_IN_io, INPUT); /* Inputs that triger HIGH , as PIR sensors*/
    }
    _lastinput = digitalRead(_IN_io);
}
void timeOUTSwitch::looper()
{
    _input_looper();
    _TOlooper();
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
void timeOUTSwitch::start_TO(int _TO, uint8_t src, bool minutes)
{
    /* Trig Types:
    ~~~~~~~~~~~~~~
    0: momentary button; 1: O-OFF switch; 2: trigger/ sensor; 3: PWM pulse counter / PW; 4: Add time each press + long press to end
   src:
   ~~~~
   0: buttons, 1: timeout, 2:MQTT cmd, 3:poweron
   */
    TO_duration = _TO; // given in seconds
    if (minutes)
    {
        TO_duration *= 60; // in case given in minutes
    }
    TO_duration /= 10; // for test purposes

    if (!inTO || src == 2)
    {
        TO_start_millis = millis();
        _updateStartClk(_now());
        delay(50);
        updateEndClk(TO_duration, _now());
        delay(50);
        _startf(src, icount);
        inTO = true;
    }
    else
    {
        if (trigType == 3) /* PWM update to inc intensity */
        {
            _startf(src, icount);
        }
        else if (trigType == 2)
        {
            TO_start_millis = millis();
            updateEndClk(TO_duration, _now());
        }
    }
}
void timeOUTSwitch::add_TO(int _TO, uint8_t src, bool minutes)
{
    if (!inTO)
    {
        start_TO(_TO, src, minutes);
    }
    else
    {
        if (minutes)
        {
            _TO *= 60; // in case given in minutes
        }
        int oldT = TO_duration;
        TO_duration = _TO; // Passing amount of time added to _startf
        _startf(src, icount);
        TO_duration = _TO + oldT; // update timeout
        updateEndClk(TO_duration, onClk());
    }
}
void timeOUTSwitch::finish_TO(uint8_t src)
{
    _endf(src, icount); /*calling first to get remTime correct on MQTT msg */
    clearTO();
}

void timeOUTSwitch::clearTO()
{
    inTO = false;
    TO_start_millis = 0;
    pCounter = 0; /* Relevant PWM only */
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyEnd, 0);   // Update endClk
        CLKstore.setValue(_keyStart, 0); // update startClk
        CLKstore.setValue(_keyCounter, 0); // update startClk
        CLKstore.printFile();
    }
}
int timeOUTSwitch::remTime()
{
    if (!_useSavedCLK)
    {
        if (TO_start_millis != 0)
        {
            int timepassed = (int)((millis() - TO_start_millis) / 1000);
            return TO_duration - timepassed;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        long bb = 0;
        CLKstore.getValue(_keyEnd, bb);
        if (bb > 0 && bb - _now() > 0)
        {
            return bb - _now();
        }
        else
        {
            return 0;
        }
    }
}
time_t timeOUTSwitch::onClk()
{
    if (_useSavedCLK)
    {
        long bb = 0;
        CLKstore.getValue(_keyStart, bb);
        return bb;
    }
    else
    {
        return _now() - (long)((millis() - TO_start_millis) / 1000);
    }
}
uint8_t timeOUTSwitch::getCount()
{
    if (_useSavedCLK)
    {
        int a = 0;
        CLKstore.getValue(_keyCounter, a);
        return (uint8_t)a;
    }
    else
    {
        return pCounter;
    }
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
    if (useInput)
    {
        bool currentRead_0 = digitalRead(_IN_io);
        delay(50);
        bool currentRead_1 = digitalRead(_IN_io);
        bool validInput = currentRead_0 == currentRead_1;

        if (trigType == 0 && validInput && currentRead_0 == _inputstatOn) /* Button - NO PWM output*/
        {
            if (inTO) /* Trun OFF*/
            {
                finish_TO(0);
            }
            else // Turn ON
            {
                start_TO(def_TO_minutes, 0);
            }

            while (digitalRead(_IN_io) == _inputstatOn) // Avoid long press
            {
                delay(10);
            }
        }
        else if (trigType == 1 && validInput && _lastinput != currentRead_0) /* Rocker Switch */
        {
            if (currentRead_0 == _inputstatOn)
            {
                start_TO(def_TO_minutes, 0);
            }
            else
            {
                finish_TO(0);
            }
            _lastinput = currentRead_0;
        }
        else if (trigType == 2 && validInput && currentRead_0 == _inputstatOn) // sensor input
        {
            if (_lastPress == 0 || (millis() - _lastPress > 1000 * 60UL)) /* Case of sensor that each detection restarts its timeout */
            {
                start_TO(def_TO_minutes, 0);
                _lastPress = millis(); /* Avoid frequent write to flash */
            }
        }
        else if (trigType == 3 && validInput && currentRead_0 == _inputstatOn) // PWM input
        {
            const uint8_t press_to_off = 3; // seconds. after this re-press PWM will set OFF

            bool cond_a = (pCounter == 0) || (pCounter < max_pCount && millis() - _lastPress < 1000 * press_to_off);              /* first press on, or inc intensity */
            bool cond_b = (inTO || pCounter >= max_pCount) || (_lastPress != 0 && (millis() - _lastPress > 1000 * press_to_off)); /* when press is far from last - turn off */

            if (cond_a) /* Turn ON*/
            {
                pCounter++;
                _lastPress = millis();
                start_TO(def_TO_minutes, 0);
            }
            else if (cond_b) /* Trun OFF*/
            {
                finish_TO(0);
                _lastPress = 0;
            }
        }
        else if (trigType == 4 && validInput && currentRead_0 == _inputstatOn) // multiPress + Long press
        {
            _lastPress = millis();
            while (digitalRead(_IN_io) == _inputstatOn) // Avoid long press
            {
                delay(10);
            }
            if (millis() - _lastPress < 700)
            {
                add_TO(def_TO_minutes, 0);
            }
            else
            {
                finish_TO(0);
            }
        }
    }
}
void timeOUTSwitch::updateEndClk(int _TO_dur, unsigned long TO_start_clk)
{
    if (TO_start_clk == 0)
    {
        TO_start_clk = _now();
    }

    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyEnd, (long)(TO_start_clk + _TO_dur));
        CLKstore.setValue(_keyCounter, (int)pCounter);
        CLKstore.printFile();
    }
}
void timeOUTSwitch::_updateStartClk(unsigned long TO_start_clk)
{
    if (_useSavedCLK)
    {
        CLKstore.setValue(_keyStart, (long)TO_start_clk);
        CLKstore.printFile();
    }
}
void timeOUTSwitch::_chk_rem_after_boot()
{
    long bb = 0;
    if (_useSavedCLK)
    {
        bool record = CLKstore.getValue(_keyEnd, bb);

        if (record)
        {
            time_t NOW = _now();
            if (bb > 0 && bb - _now() > 0 && NOW > 1627735850)
            {
                start_TO(bb - NOW, 1, false);
            }
            else if (bb > 0 && bb - NOW < 0)
            {
                clearTO();
            }
        }
        else
        {
            clearTO();
        }
    }
}
time_t timeOUTSwitch::_now()
{
    return time(nullptr);
}
// uint8_t timeOUTSwitch::_counter = 0;
