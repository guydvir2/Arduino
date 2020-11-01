#include "Arduino.h"
#include "esp8266Sleep.h"

// ~~~~~~~~ Start ~~~~~~~~~
esp8266Sleep::esp8266Sleep() : FVAR_bootClock("currentBoot"),
                               FVAR_bootCounter("bootCounter"),
                               FVAR_nextWakeClock("nextWake")
{
}
void esp8266Sleep::start(int deepsleep, int forcedwake, char *devname, cb_func wake_cb, cb_func sleep_cb)
{
    _wake_cb = wake_cb;
    _sleep_cb = sleep_cb;
    _deepsleep = deepsleep;
    _forcedWake = forcedwake;
    sprintf(_devname, "%s", devname);
    clock_update_success = after_wakeup_clockupdates();
    onWake_cb();
}
void esp8266Sleep::delay_sleep(int sec_delay)
{
    _start_delay = true;
    sec_wait = sec_delay;
    _delay_counter = millis();
}
void esp8266Sleep::nextSleepCalculation()
{
    time_t t = now();

    nextsleep_duration = _deepsleep * MINUTES - (minute(t) * 60 + second(t)) % (_deepsleep * MINUTES);
    FVAR_nextWakeClock.setValue(t + nextsleep_duration);
}
void esp8266Sleep::gotoSleep(int seconds2sleep)
{
    if(!isESP32){
    Serial.printf("Going to sleep for %d [sec]", seconds2sleep);
    Serial.flush();
    delay(200);
    ESP.deepSleep(microsec2sec * seconds2sleep);
    }
}
void esp8266Sleep::wait2Sleep()
{
    if (_start_delay == false)
    {
        if (clock_update_success)
        {
            if (drift < 0 && drift > -1 * _sec_to_ignore_wake_before_time) /* wake up up to 30 sec earlier */
            {
                if (millis() > (_forcedWake + abs(drift)) * 1000)
                {
                    nextSleepCalculation();
                    Serial.print("missed wake up by: ");
                    Serial.println(drift);

                    Serial.print("drift correction is: ");
                    Serial.println((float)nextsleep_duration * (driftFactor));
                    Serial.flush();
                    if (_sleep_cb != nullptr)
                    {
                        _sleep_cb();
                    }
                    gotoSleep(nextsleep_duration * driftFactor);
                }
            }
            else if (drift < -1 * _sec_to_ignore_wake_before_time) /* wake up more than 30 sec earlier */
            {
                if (millis() > _sec_to_wait_big_drift * 1000)
                {
                    Serial.print("missed wake up by: ");
                    Serial.println(drift);
                    Serial.println("going to sleep early");
                    Serial.flush();
                    if (_sleep_cb != nullptr)
                    {
                        _sleep_cb();
                    }
                    nextsleep_duration = abs(drift - _sec_to_wait_big_drift);
                    gotoSleep(nextsleep_duration);
                }
            }
            else /* wake up after time - which is OK... sort of */
            {
                if (millis() > _forcedWake * 1000)
                {
                //     Serial.print("missed wake up by: ");
                //     Serial.println(drift);
                //     Serial.print("drift correction is: ");
                //     Serial.println((float)nextsleep_duration * (driftFactor));
                //     Serial.flush();

                    nextSleepCalculation();
                    if (_sleep_cb != nullptr)
                    {
                        _sleep_cb();
                    }
                    gotoSleep(nextsleep_duration * driftFactor);
                }
            }
        }
        else
        {
            if (millis() > _forcedWake * 1000)
            {
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                gotoSleep(_deepsleep * MINUTES * driftFactor);
            }
        }
    }
    else
    { // OTA
        if (millis() > _delay_counter + sec_wait * 1000 && millis() > _forcedWake * 1000)
        {
            if (clock_update_success)
            {
                nextSleepCalculation();
                Serial.print("missed wake up by: ");
                Serial.println(drift);

                Serial.print("drift correction is: ");
                Serial.println((float)nextsleep_duration * (driftFactor));
                Serial.flush();
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                gotoSleep(nextsleep_duration * driftFactor);
            }
            else
            {
                Serial.println("NO_NTP");
                Serial.flush();
                if (_sleep_cb != nullptr)
                {
                    _sleep_cb();
                }
                gotoSleep(_deepsleep * MINUTES * driftFactor);
            }
        }
    }
}
void esp8266Sleep::onWake_cb()
{
    if (_wake_cb != nullptr)
    {
        _wake_cb();
    }
}
bool esp8266Sleep::after_wakeup_clockupdates()
{
    time_t lastBoot = 0;
    time_t last_wakeClock = 0;
    wakeClock = now();

    FVAR_bootClock.getValue(lastBoot);
    FVAR_nextWakeClock.getValue(last_wakeClock);
    FVAR_bootCounter.getValue(bootCount);
    FVAR_bootCounter.setValue(++bootCount);

    if (year(wakeClock) != 1970)
    {
        FVAR_bootClock.setValue(wakeClock);
        totalSleepTime = wakeClock - lastBoot - _forcedWake;
        drift = wakeClock - last_wakeClock - (int)(millis() / 1000);
        return 1;
    }
    else
    {
        return 0;
    }
}
