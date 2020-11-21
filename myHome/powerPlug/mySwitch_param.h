mySwitch *TOswitches[1] = {};

// ~~~ mySwitch functions ~~~~~~~~
void TOswitch_init()
{
    static mySwitch myTOsw0;
    TOswitches[0] = &myTOsw0;

    for (int i = 0; i < numSW; i++)
    {
        TOswitches[i]->usePWM = usePWM;
        TOswitches[i]->useSerial = sketchJSON["useSerial"];
        TOswitches[i]->useInput = sketchJSON["useInput"];
        TOswitches[i]->useEXTtrigger = sketchJSON["useExtTrig"];
        TOswitches[i]->useHardReboot = sketchJSON["useEEPROM_resetCounter"];
        TOswitches[i]->is_momentery = sketchJSON["momentryButtorn"];
        TOswitches[i]->badBoot = sketchJSON["useResetKeeper"];
        TOswitches[i]->useDailyTO = sketchJSON["usedailyTO"];
        TOswitches[i]->usesafetyOff = sketchJSON["useSafteyOff"];
        TOswitches[i]->set_safetyoff = sketchJSON["safetyOffDuration"];
        TOswitches[i]->usequickON = sketchJSON["usequickBoot"];
        TOswitches[i]->onAt_boot = sketchJSON["useOnatBoot"];
        TOswitches[i]->def_power = sketchJSON["defPWM"];
        TOswitches[i]->usetimeOUT = sketchJSON["usetimeOUT"];
        TOswitches[i]->inputState = sketchJSON["inputState"];
        TOswitches[i]->inputPin = inputPin[i];

        TOswitches[i]->config(outputPin[i], sketchJSON["timeOUTS"][i], SW_Names[i]);
    }
}
void startdTO()
{
    // After Wifi is On
    int START_dTO[maxSW][3];
    int END_dTO[maxSW][3];

    for (int a = 0; a < numSW; a++)
    {
        for (int i = 0; i < 3; i++)
        {
            START_dTO[a][i] = sketchJSON["start_dTO"][a][i].as<int>();
            END_dTO[a][i] = sketchJSON["end_dTO"][a][i].as<int>();
        }
    }

    for (int i = 0; i < numSW; i++)
    {
        if (TOswitches[i]->useDailyTO)
        {
            TOswitches[i]->setdailyTO(START_dTO[i], END_dTO[i]);
        }
        if (TOswitches[i]->useEXTtrigger)
        {
            TOswitches[i]->extTrig_cb(HIGH, true, "PIR_detector");
        }
        TOswitches[i]->begin();
    }
    if (useExtTrig)
    {
        // startExtTrig();
    }
}
void TOswitch_looper()
{
    char msgtoMQTT[150];
    byte mtyp;

    for (int i = 0; i < numSW; i++)
    {
        TOswitches[i]->looper(iot.mqtt_detect_reset);
        if (TOswitches[i]->postMessages(msgtoMQTT, mtyp))
        {
            if (mtyp == 0)
            {
                iot.pub_msg(msgtoMQTT);
            }
            if (mtyp == 1)
            {
                iot.pub_log(msgtoMQTT);
            }
            if (mtyp == 2)
            {
                iot.pub_state(msgtoMQTT, i);
            }
        }
    }
    // if (useExtTrig)
    // {
    //     readExtTrig_looper();
    // }
}
