#include "Arduino.h"
#include "esp8266Sleep.h"

// ~~~~~~~~ Start ~~~~~~~~~
esp8266Sleep::esp8266Sleep() : FVAR_bootClock("currentBoot"),
                               FVAR_bootCounter("bootCounter"),
                               FVAR_nextWakeClock("nextWake")
{
}
void esp8266Sleep::start(int deepsleep, int forcedwake, char *devname, cb_func ext_cb)
{
    _ext_cb = ext_cb;
    _deepsleep = deepsleep;
    _forcedWake = forcedwake;
    sprintf(_devname, "%s", devname);
    clock_update_success = after_wakeup_clockupdates();
}
void esp8266Sleep::init_OTA()
{
    _start_ota = true;
    _ota_counter = millis();
}
// prepare to sleep~~~~
void esp8266Sleep::nextSleepCalculation()
{
    time_t t = now();

    nextsleep_duration = _deepsleep * MINUTES - (minute(t) * 60 + second(t)) % (_deepsleep * MINUTES);
    nextsleep_duration = nextsleep_duration;
    FVAR_nextWakeClock.setValue(t + nextsleep_duration);
}
void esp8266Sleep::gotoSleep(int seconds2sleep)
{
    Serial.printf("Going to sleep for %d [sec]", seconds2sleep);
    Serial.flush();
    delay(200);
    ESP.deepSleep(microsec2sec * seconds2sleep);
}
void esp8266Sleep::wait2Sleep()
{
    if (_start_ota == false)
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
                    gotoSleep(abs(drift - _sec_to_wait_big_drift));
                }
            }
            else /* wake up after time - which is OK... sort of */
            {
                if (millis() > _forcedWake * 1000)
                {
                    nextSleepCalculation();
                    Serial.print("missed wake up by: ");
                    Serial.println(drift);

                    Serial.print("drift correction is: ");
                    Serial.println((float)nextsleep_duration * (driftFactor));
                    Serial.flush();

                    gotoSleep(nextsleep_duration * driftFactor);
                }
            }
        }
        else
        {
            if (millis() > _forcedWake * 1000)
            {
                Serial.println("NO_NTP");
                Serial.flush();
                gotoSleep(_deepsleep * MINUTES * driftFactor);
            }
        }
    }
    else
    { // OTA
        if (millis() > _ota_counter + sec_waitOTA * 1000 && millis() > _forcedWake * 1000)
        {
            if (clock_update_success)
            {
                nextSleepCalculation();
                Serial.print("missed wake up by: ");
                Serial.println(drift);

                Serial.print("drift correction is: ");
                Serial.println((float)nextsleep_duration * (driftFactor));
                Serial.flush();

                gotoSleep(nextsleep_duration * driftFactor);
            }
            else
            {
                Serial.println("NO_NTP");
                Serial.flush();
                gotoSleep(_deepsleep * MINUTES * driftFactor);
            }
        }
    }
}
// Wake functions
void esp8266Sleep::onWake_cb()
{
    _ext_cb();
}
bool esp8266Sleep::after_wakeup_clockupdates()
{
    wakeClock = now();
    time_t lastBoot = 0;
    time_t last_wakeClock = 0;

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
