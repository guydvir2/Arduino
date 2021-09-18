#include "Arduino.h"
#include "sleepyESP.h"

// ~~~~~~~~ Start ~~~~~~~~~
sleepyESP::sleepyESP()
{
#if isESP32
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector for bat operations
#endif
    yield;
}
void sleepyESP::start(uint8_t deepsleep_mins, uint8_t forcedwake_secs, char *devname, cb_func wake_cb, cb_func sleep_cb, bool clkAlign)
{
    EEPROM.begin(100);
    _wake_cb = wake_cb;
    _sleep_cb = sleep_cb;
    _deepsleep_mins = deepsleep_mins;
    _nominal_wait_secs = forcedwake_secs;
    _clkAlign = clkAlign;
    sprintf(_devname, "%s", devname);
    clock_update_success = post_wake_clkUpdate();
    _onWake_cb();
}
void sleepyESP::wait2Sleep() /* place in loop at main code */
{
    if (_force_postpone_sleep == false) /* Force postpone sleep - for OTA purposes */
    {
        if (clock_update_success && _clkAlign == true)
        {
            if (wake_up_drift_sec >= 0)
            {
                if (millis() > _nominal_wait_secs * 1000)
                {
                    Serial.print("a) After time wake up");
                    Serial.flush();

                    _calc_nextSleep_duration();
                    if (_sleep_cb != nullptr)
                    {
                        _sleep_cb();
                    }
                    _gotoSleep(nextsleep_duration * _driftFactor);
                }
            }
            else if (abs(wake_up_drift_sec) < _allowed_wake_err_sec) /* wake up earlier up to [x] sec */
            {
                if (millis() > _nominal_wait_secs * 1000) /* Completing wake cycle */
                {
                    Serial.println("b) Minor pre-time wakeup");
                    Serial.flush();
                    if (_sleep_cb != nullptr)
                    {
                        _sleep_cb();
                    }
                    Serial.print("millis_diff: ");
                    Serial.println(millis() / 1000 - abs(wake_up_drift_sec));
                    if (millis() > abs(wake_up_drift_sec) * 1000)
                    {
                        _calc_nextSleep_duration();
                        _gotoSleep(nextsleep_duration * _driftFactor);
                    }
                    else
                    {
                        int tsleep = millis() / 1000 - abs(wake_up_drift_sec) + _deepsleep_mins * MINUTES2SEC;
                        _saveNext_wakeup(tsleep);
                        _gotoSleep(tsleep * _driftFactor);
                    }
                }
            }
            else if (abs(wake_up_drift_sec) > _allowed_wake_err_sec) /* wake up more than 30 sec earlier */
            {
                Serial.println("c) Major pre-time wakeup.");
                Serial.flush();
                Serial.print("Drift: ");
                Serial.println(wake_up_drift_sec);
                int tsleep = abs(wake_up_drift_sec) - millis() / 1000;
                _saveNext_wakeup(tsleep);
                _gotoSleep(tsleep * _driftFactor);
            }
        }
        else /* Clk is not setup - just wait nominal time */
        {
            if (millis() > _nominal_wait_secs * 1000)
            {
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                Serial.println("not Aligned");
                _gotoSleep(_deepsleep_mins * MINUTES2SEC * _driftFactor);
            }
        }
    }
    else
    { // OTA
        if (millis() > _force_postpone_millis + force_postpone_sec * 1000 && millis() > _nominal_wait_secs * 1000)
        {
            if (clock_update_success)
            {
                _calc_nextSleep_duration();
                Serial.print("missed wake up by: ");
                Serial.println(wake_up_drift_sec);

                Serial.print("wake_up_drift_sec correction is: ");
                Serial.println((float)nextsleep_duration * (_driftFactor));
                Serial.flush();
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                _gotoSleep(nextsleep_duration * _driftFactor);
            }
            else
            {
                Serial.println("NO_NTP");
                Serial.flush();
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                _gotoSleep(_deepsleep_mins * MINUTES2SEC * _driftFactor);
            }
        }
    }
}
bool sleepyESP::post_wake_clkUpdate() /* Update clock after wakeup */
{
    time_t nowClk = time(nullptr);
    struct tm *timeinfo = localtime(&nowClk);

    time_t lastBoot = EEPROMReadlong(_bootClock_addr);
    time_t last_wakeClock = EEPROMReadlong(_nextWake_clock_addr);
    bootCount = EEPROMReadlong(_bootCounter_addr) + 1;
    EEPROMWritelong(_bootCounter_addr, bootCount);

    if (timeinfo->tm_year != 70)
    {
        EEPROMWritelong(_bootClock_addr, nowClk);
        totalSleepTime = nowClk - lastBoot - _nominal_wait_secs;
        wake_up_drift_sec = nowClk - last_wakeClock - (int)(millis() / 1000);
        if (abs(wake_up_drift_sec) > _deepsleep_mins * MINUTES2SEC)
        {
            wake_up_drift_sec = 0;
            Serial.println("drift_err. value zeroed");
        }
        return 1;
    }
    else
    {
        wake_up_drift_sec = 0;
        return 0;
    }
}
void sleepyESP::update_sleep_duration(int sleep_time) /* External sleep update ( as in MQTT cmd )*/
{
    _deepsleep_mins = sleep_time;
}
void sleepyESP::delay_sleep(int sec_delay) /* Force time to postpone nominal sleep */
{
    _force_postpone_sleep = true;
    force_postpone_sec = sec_delay;
    _force_postpone_millis = millis();
}

void sleepyESP::_onWake_cb()
{
    if (_wake_cb != nullptr)
    {
        _wake_cb();
    }
}
void sleepyESP::_calc_nextSleep_duration()
{
    time_t t = time(nullptr);
    struct tm *timeinfo = localtime(&t);

    nextsleep_duration = _deepsleep_mins * MINUTES2SEC - (timeinfo->tm_min * 60 + timeinfo->tm_sec) % (_deepsleep_mins * MINUTES2SEC);
    _saveNext_wakeup(nextsleep_duration);
    // EEPROMWritelong(_nextWake_clock_addr, t + nextsleep_duration);
}
void sleepyESP::_gotoSleep(int seconds2sleep)
{
    Serial.printf("Sleep cycle %d[sec], ", _deepsleep_mins * 60);
    Serial.printf("Calulated sleep %d[sec], ", nextsleep_duration);
    Serial.printf("error-corrected sleep %d[sec]", seconds2sleep);
    Serial.flush();
    delay(100);
#if isESP8266
    ESP.deepSleep(microsec2sec * seconds2sleep);
#elif isESP32
    esp_sleep_enable_timer_wakeup(seconds2sleep * microsec2sec);
    esp_deep_sleep_start();
#endif
}
void sleepyESP::_saveNext_wakeup(int duration, time_t t)
{
    EEPROMWritelong(_nextWake_clock_addr, t + duration);
}
void sleepyESP::EEPROMWritelong(int address, long value)
{
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);

    EEPROM.write(address, four);
    EEPROM.write(address + 1, three);
    EEPROM.write(address + 2, two);
    EEPROM.write(address + 3, one);
    EEPROM.commit();
}
long sleepyESP::EEPROMReadlong(long address)
{
    long four = EEPROM.read(address);
    long three = EEPROM.read(address + 1);
    long two = EEPROM.read(address + 2);
    long one = EEPROM.read(address + 3);

    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}