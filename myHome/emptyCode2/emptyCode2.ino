#include <myIOT2.h>
#include "empty_param.h"
#include "myIOT_settings.h"
#include <Arduino.h>

void setup()
{
        // startRead_parameters();
        startIOTservices();
        // endRead_parameters();
        // iot.pub_email("g","fhfhfjh","yybyyb");
}
void loop()
{
        iot.looper();
        delay(100);
}
