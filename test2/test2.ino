#include <myIOT2.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif

#include "myIOT_settings.h"
#include "fireBase_settings.h"

void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        startIOTservices();
#endif
fireBase_init();
}

void loop()
{
        iot.looper();
        fireBaseloop();
}
