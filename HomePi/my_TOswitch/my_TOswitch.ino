
#include <myIOT.h>
#include "SWITCH_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "ESP01_2.3"
bool usePWM;
bool useExtTrig;
int numSW = 2; // changes after reading JSON param

// ********** myIOT Class ***********
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;

// ~~~~ TO & dailyTO ~~~~~~
mySwitch *TOswitches[maxSW] = {};

// ~~~~~~ Hardware ~~~~~~~
int outputPin[maxSW];
int inputPin[maxSW];
int extTrigPin;
int hRebbots[maxSW];
char SW_Names[maxSW][30];

//~~ extTrig functions
void startExtTrig()
{
	pinMode(extTrigPin, INPUT);
}
void readExtTrig_looper(int a = 0)
{
	TOswitches[a]->ext_trig_signal = digitalRead(extTrigPin);
}

void configTOswitches()
{
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
		TOswitches[i]->inputPin = inputPin[i];
		if (sketchJSON["useEEPROM_resetCounter"])
		{
			TOswitches[i]->hReboot.check_boot(hRebbots[i]);
		}
		if (sketchJSON["usequickBoot"])
		{
			TOswitches[i]->quickPwrON();
		}
		TOswitches[i]->config(outputPin[i], sketchJSON["timeOUTS"][i], SW_Names[i]);
	}
}
void TOswitch_init()
{
	static mySwitch myTOsw0;
	TOswitches[0] = &myTOsw0;

	if (numSW == 2)
	{
		static mySwitch myTOsw1;
		TOswitches[1] = &myTOsw1;
	}
	configTOswitches();
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
	if (useExtTrig)
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
	iot.useNetworkReset = paramJSON["useNetworkReset"];
	iot.noNetwork_reset = paramJSON["noNetwork_reset"];
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
		sprintf(msg, "ver #2: [%s], useSafeyOff[%d], safetyDuration[%d]", VER, TOswitches[0]->usesafetyOff, TOswitches[0]->set_safetyoff);
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
		TOswitches[0]->TOswitch.inCodeTimeOUT_inFlash.printFile();
	}
	else if (strcmp(incoming_msg, "format") == 0)
	{
		TOswitches[0]->TOswitch.inCodeTimeOUT_inFlash.format();
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

void setup()
{
	startRead_parameters();
	TOswitch_init();
	startIOTservices();
	startdTO();
	endRead_parameters();
}
void loop()
{
	iot.looper();
	TOswitch_looper();
	delay(100);
}
