#include <myIOT2.h>
#include <myPIR.h>
#include <Arduino.h>

/*
 on ESP-01
 TX - GPIO1 --> OUTPUT ONLY
 RX - GPIO3 --> INPUT  ONLY
 */

// ********** Sketch Services  ***********
#define VER "ESP-01_1.5"

#define Pin_Sensor_0 4
#define Pin_Switch_0 2
#define Pin_extbut_0 13 // fake
#define SwitchTimeOUT_0 60 //minutes

#define Pin_Switch_1 1
#define Pin_Sensor_1 13 // fake io - not using sensor
#define Pin_extbut_1 3  // using button to switch on/ off
#define SwitchTimeOUT_1 30 //minutes

/*
 #define Pin_Sensor_0 16 //
 #define Pin_Sensor_1 16 // not using sensor
 #define Pin_Switch_0 D1
 #define Pin_Switch_1 D3
 #define Pin_extbut_0 D5 // using button to switch on/ off
 #define Pin_extbut_1 D7 // using button to switch on/ off
 */

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amount of fail clock updates
#define USE_DEBUG true
#define DEBUG_LEVEL 0
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "test"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT2 iot;
//(DEVICE_TOPIC);
// ***************************

// ±±±±±±±±±±±±±± mySwitch ±±±±±±±±±±±±±±±
#define NUM_SW 1
const char *ledNames[] = { "LEDstrip", "empty" };
SensorSwitch s0(Pin_Sensor_0, Pin_Switch_0, SwitchTimeOUT_0, Pin_extbut_0);

#if NUM_SW == 1
SensorSwitch *s[NUM_SW] = { &s0 };
#else if NUM_SW == 2
SensorSwitch s1(Pin_Sensor_1, Pin_Switch_1, SwitchTimeOUT_1, Pin_extbut_1);
SensorSwitch *s[NUM_SW] = {&s0, &s1};
#endif

void startIOTservices() {
	iot.useSerial = USE_SERIAL;
	iot.useWDT = USE_WDT;
	iot.useOTA = USE_OTA;
	iot.useResetKeeper = USE_RESETKEEPER;
	iot.resetFailNTP = USE_FAILNTP;
	iot.useDebug = USE_DEBUG;
	iot.debug_level = DEBUG_LEVEL;
	strcpy(iot.prefixTopic, MQTT_PREFIX);
	strcpy(iot.addGroupTopic, MQTT_GROUP);
	strcpy(iot.deviceTopic, DEVICE_TOPIC);
	iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg) {
	char msg[150];
	char msg2[20];

	if (strcmp(incoming_msg, "status") == 0) {
		sprintf(msg, "Status:");
		for (int i = 0; i < NUM_SW; i++) {
			if (s[i]->swState < 1.0 && s[i]->swState > 0.0) {
				sprintf(msg2, "[%s] [%.1f%%] [On] ", ledNames[i],
						s[i]->swState * 100);
			} else {
				sprintf(msg2, "[%s] [%s] ", ledNames[i],
						s[i]->swState ? "On" : "Off");
			}
			strcat(msg, msg2);
		}

		iot.pub_msg(msg);
	} else if (strcmp(incoming_msg, "ver2") == 0) {
		sprintf(msg, "ver2: [%s]", VER);
		iot.pub_msg(msg);
	} else if (strcmp(incoming_msg, "help2") == 0) {
		sprintf(msg,
				"Help: Commands #3 - [all_off; i,on; i,off; i,startTO,mins; remain]");
		iot.pub_msg(msg);
	} else if (strcmp(incoming_msg, "all_off") == 0) {
		for (int i = 0; i < NUM_SW; i++) {
			if (s[i]->swState != 0.0) {
				s[i]->turnOff();
				sprintf(msg, "MQTT: [%s] Turned [Off]", ledNames[i]);
				iot.pub_msg(msg);
			}
		}
	} else if (strcmp(incoming_msg, "remain") == 0) {
		for (int i = 0; i < NUM_SW; i++) {
			if (s[i]->timeoutRem > 0) {
				sprintf(msg, "MQTT: Remain Time LedStrip [%s] ,[%d] sec",
						ledNames[i], s[i]->timeoutRem);
				iot.pub_msg(msg);
			} else {
				sprintf(msg, "MQTT: LedStrip [%s] is [Off]", ledNames[i]);
				iot.pub_msg(msg);
			}
		}
	}

	else {
		int num_parameters = iot.inline_read(incoming_msg);
		int x = atoi(iot.inline_param[0]);

		if (strcmp(iot.inline_param[1], "on") == 0 && x < NUM_SW) {
			s[x]->turnOn();
		} else if (strcmp(iot.inline_param[1], "off") == 0 && x < NUM_SW) {
			s[x]->turnOff();
		}
		if (strcmp(iot.inline_param[1], "startTO") == 0 && x < NUM_SW) {
			s[x]->turnOn(atoi(iot.inline_param[2]));
		}
	}
}
void notifyMQTT() {
	static float lastval[] = { 0.0, 0.0, 0.0 };
	char msg[50];

	for (int i = 0; i < NUM_SW; i++) {
		if (s[i]->swState != lastval[i]) {
			lastval[i] = s[i]->swState;
			if (s[i]->usePWM) {
				sprintf(msg, "Change: [%s] changed to [%.1f]", ledNames[i],
						s[i]->swState);
			} else {
				sprintf(msg, "Change: [%s] is now [%s]", ledNames[i],
						s[i]->swState ? "On" : "Off");
			}
			iot.pub_msg(msg);
		}
	}
}

void setup() {
	s0.useButton = false;
	s0.usePWM = true;
	s0.RelayON_def = true;
	s0.ButtonPressed_def = LOW;
	s0.SensorDetection_def = HIGH;
	s0.start();
#if NUM_SW == 2
        s1.useButton = true;
        s1.usePWM = false;
        s1.RelayON_def = true;
        s1.ButtonPressed_def = LOW;
        s1.SensorDetection_def = LOW;
        s1.start();
#endif

	startIOTservices();
}

void loop() {
	iot.looper();
	s0.looper();
#if NUM_SW == 2
        s1.looper();
#endif
	notifyMQTT();
	delay(100);
}
