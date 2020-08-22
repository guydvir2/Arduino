#include <myIOT.h>
#include "SWITCH_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "ESP01_2.0"
#define USE_DISPLAY false
#define numSW 1

// ********** myIOT Class ***********
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;

// ~~~~ TO & dailyTO ~~~~~~
int START_dTO[2][3];
int END_dTO[2][3];
int TimeOUT[2];

// ~~~~~~ Hardware ~~~~~~~
bool usePWM;
int outputPin[2];
int inputPin[2];	  
int extTrigPin;		  
int hRebbots[2];	  
char SW_Names[2][30]; 

mySwitch myTOsw0(outputPin[0], TimeOUT[0], SW_Names[0]);
#if numSW == 2
mySwitch myTOsw1(outputPin[1], TimeOUT[1], SW_Names[1]);
mySwitch *TOswitches[numSW] = {&myTOsw0, &myTOsw1};
#elif numSW == 1
mySwitch *TOswitches[numSW] = {&myTOsw0};
#endif

//~~ extTrig functions
void startExtTrig()
{
	pinMode(extTrigPin, INPUT);
}
void readExtTrig_looper()
{
	TOswitches[0]->ext_trig_signal = digitalRead(extTrigPin);
}

void configTOswitches()
{
	for (int i = 0; i < numSW; i++)
	{
		TOswitches[i]->usePWM = usePWM;
		TOswitches[i]->useSerial = paramJSON["useSerial"];
		TOswitches[i]->useInput = paramJSON["useInput"];
		TOswitches[i]->useEXTtrigger = paramJSON["useExtTrig"];
		TOswitches[i]->useHardReboot = paramJSON["useEEPROM_resetCounter"];
		TOswitches[i]->is_momentery = paramJSON["momentryButtorn"];
		TOswitches[i]->badBoot = paramJSON["useResetKeeper"];
		TOswitches[i]->useDailyTO = paramJSON["usedailyTO"];
		TOswitches[i]->usesafetyOff = paramJSON["useSafteyOff"];
		TOswitches[i]->set_safetyoff = paramJSON["safetyOffDuration"];
		TOswitches[i]->usequickON = paramJSON["usequickBoot"];
		TOswitches[i]->onAt_boot = paramJSON["useOnatBoot"];
		TOswitches[i]->inputPin = inputPin[i];

		if (paramJSON["useEEPROM_resetCounter"])
		{
			TOswitches[i]->hReboot.check_boot(hRebbots[i]);
		}
		if (paramJSON["usequickBoot"])
		{
			TOswitches[i]->quickPwrON();
		}
	}
}
void startTOSwitch()
{
	// After Wifi is On
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
	if (paramJSON["useExtTrig"])
	{
		startExtTrig();
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
	if (paramJSON["useExtTrig"])
	{
		readExtTrig_looper();
	}
}

// ***********************************
void startIOTservices()
{
	iot.useSerial = paramJSON["useSerial"];
	iot.useWDT = paramJSON["useWDT"];
	iot.useOTA = paramJSON["useOTA"];
	iot.useResetKeeper = paramJSON["useResetKeeper"];
	iot.resetFailNTP = paramJSON["useFailNTP"];
	iot.useDebug = paramJSON["useDebugLog"];
	iot.debug_level = paramJSON["debug_level"];
	;
	strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
	strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
	strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
	iot.start_services(ADD_MQTT_FUNC);
}
void giveStatus(char *outputmsg)
{
	char t1[50];
	char t2[50];
	char t3[150];

	sprintf(t3, "Status: ");
	for (int i = 0; i < numSW; i++)
	{
		if (TOswitches[i]->TOswitch.remain() > 0)
		{
			TOswitches[i]->TOswitch.convert_epoch2clock(
				now() + TOswitches[i]->TOswitch.remain(), now(), t2, t1);
			sprintf(t1, "timeLeft[%s]", t2);
		}
		else
		{
			sprintf(t1, "");
		}
		if (usePWM)
		{
			if (TOswitches[i]->current_power == 0)
			{
				sprintf(t2, "[%s] [OFF] %s ", SW_Names[i], t1);
			}
			else
			{
				sprintf(t2, "[%s] power[%.0f%%] %s ", SW_Names[i],
						TOswitches[i]->current_power * 100, t1);
			}
		}
		else
		{
			sprintf(t2, "[%s] [%s] %s ", SW_Names[i],
					TOswitches[i]->current_power ? "ON" : "OFF", t1);
		}
		strcat(t3, t2);
	}
	sprintf(outputmsg, "%s", t3);
}
void all_off(char *from)
{
	for (int i = 0; i < numSW; i++)
	{
		TOswitches[i]->all_off(from);
	}
}
void addiotnalMQTT(char *incoming_msg)
{
	char msg[150];
	char msg2[20];

	if (strcmp(incoming_msg, "status") == 0)
	{
		giveStatus(msg);
		iot.pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "ver2") == 0)
	{
		sprintf(msg, "ver #2: [%s], useSafeyOff[%d], safetyDuration[%d]", VER, paramJSON["useSafteyOff"].as<int>(), paramJSON["safetyOffDuration"].as<int>());
		iot.pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "help2") == 0)
	{
		sprintf(msg,
				"Help: Commands #3 - [remain, restartTO, timeout, endTO, updateTO, restoreTO, statusTO]");
		iot.pub_msg(msg);
		sprintf(msg,
				"Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
		iot.pub_msg(msg);
		sprintf(msg,
				"Help: Commands #5 - [on, off, change_pwm, all_off, flash, format]");
		iot.pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "flash") == 0)
	{
		myTOsw0.TOswitch.inCodeTimeOUT_inFlash.printFile();
	}
	else if (strcmp(incoming_msg, "format") == 0)
	{
		myTOsw0.TOswitch.inCodeTimeOUT_inFlash.format();
	}
	else if (strcmp(incoming_msg, "all_off") == 0)
	{
		all_off("MQTT");
	}

	// ±±±±±±±±±± MQTT MSGS from mySwitch Library ±±±±±±±±±±±±
	else
	{
		int num_parameters = iot.inline_read(incoming_msg);
		TOswitches[atoi(iot.inline_param[0])]->getMQTT(iot.inline_param[1], atoi(iot.inline_param[2]), atoi(iot.inline_param[3]), atoi(iot.inline_param[4]));
		for (int n = 0; n <= num_parameters - 1; n++)
		{
			sprintf(iot.inline_param[n], "");
		}
	}
}

// ~~~~~~~~~~~~~ Display ~~~~~~~~~~~~~~~
// #if USE_DISPLAY
// #include <myDisplay.h>
// myOLED OLED(64);

// void displayClock()
// {
// 	char clock[20];
// 	char date[20];
// 	const int swapPeriod = 5000; // milli-seconds
// 	static unsigned long swapLines_counter = 0;

// 	iot.return_clock(clock);
// 	iot.return_date(date);

// 	if (millis() - swapLines_counter < swapPeriod)
// 	{
// 		OLED.CenterTXT(clock, "", "", date);
// 	}
// 	else if (millis() - swapLines_counter < 2 * swapPeriod && millis() - swapLines_counter >= swapPeriod)
// 	{
// 		OLED.CenterTXT("", clock, date, "");
// 	}
// 	else
// 	{
// 		swapLines_counter = millis();
// 	}
// }
// #endif

void setup()
{
	read_parameters_from_file();
	configTOswitches();
	startIOTservices();
	startTOSwitch();
	// #if USE_DISPLAY
	// 	OLED.start();
	// #endif
	free_paramJSON();
}
void loop()
{
	iot.looper();
	TOswitch_looper();
	// #if USE_DISPLAY
	// 	displayClock();
	// #endif
	delay(100);
}
