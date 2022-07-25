#include <myIOT2.h>
#include <timeoutButton.h>
#include <TurnOnLights.h>

myIOT2 iot;
TurnOnLights lightOutput_0;
TurnOnLights lightOutput_1;
TurnOnLights *lightOutputV[2] = {&lightOutput_0, &lightOutput_1};

timeoutButton timeoutButton_0;
timeoutButton timeoutButton_1;
timeoutButton *timeoutButtonV[2] = {&timeoutButton_0, &timeoutButton_1};

struct oper_string
{
  bool state;     /* On or Off */
  uint8_t step;   /* Step, in case of PWM */
  uint8_t reason; /* What triggered the button */
  time_t ontime;  /* Start Clk */
  time_t offtime; /* Off Clk */
};
oper_string OPERstring_0 = {false, 0, 0, 0, 0};
oper_string OPERstring_1 = {false, 0, 0, 0, 0};
oper_string *OPERstringV[2] = {&OPERstring_0, &OPERstring_1};

//  ±±±±±±±±±±±±±±± Sketch variable - updates from flash ±±±±±±±±±±±±±±±±±±
uint8_t numSW = 2;

bool OnatBoot[2] = {true, false};
bool useInput[2] = {true, true};
bool outputPWM[2] = {false, true};
bool useIndicLED[2] = {false, false};
bool dimmablePWM[2] = {true, false};

bool output_ON[2] = {HIGH, HIGH};
bool inputPressed[2] = {LOW, LOW};

uint8_t trigType[2] = {0, 1};
uint8_t inputPin[2] = {D3, D5};
uint8_t outputPin[2] = {D6, D7};
uint8_t indicPin[2] = {D4, D8};

int def_TO_minutes[2] = {300, 300};
int maxON_minutes[2] = {1000, 1000};

uint8_t defPWM[2] = {2, 2};
uint8_t max_pCount[2] = {3, 3};
uint8_t limitPWM[2] = {80, 80};
uint8_t PWM_res = 1023;

int sketch_JSON_Psize = 1250;
char sw_names[2][10] = {"led0", "led1"};
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

bool P_readOK_a = false;
bool P_readOK_b = false;
bool P_readOK_c = false;

#include "myIOT_settings.h"

// ~~~~~~~~~~~~ Operational String Read & Write ~~~~~~~~~~~~~~~

const char *operStr_filename = "/operstr.txt";

void save_OperStr(oper_string &str)
{
  File _file = LittleFS.open(operStr_filename, "w");
  _file.write((byte *)&str, sizeof(str));
  _file.close();
}
void read_OperStr(oper_string &str)
{
  File _file = LittleFS.open(operStr_filename, "r");
  _file.read((byte *)&str, sizeof(str));
  _file.close();
}
void update_OperString(uint8_t reason, uint8_t i, bool state)
{
  OPERstringV[i]->reason = reason;
  OPERstringV[i]->state = state;
  if (state == true)
  {
    OPERstringV[i]->ontime = iot.now();
    OPERstringV[i]->offtime = OPERstringV[i]->ontime + timeoutButtonV[i]->timeout * 60;
  }
  else
  {
    OPERstringV[i]->ontime = 0;
    OPERstringV[i]->offtime = iot.now();
  }
  lightOutputV[i]->isPWM() ? OPERstringV[i]->step = lightOutputV[i]->currentStep : OPERstringV[i]->step = 0;
  save_OperStr(*OPERstringV[i]);
  Serial.print("Updated OPERstring: \t#");
  Serial.println(i);
  print_OPERstring(*OPERstringV[i]);
}
void print_OPERstring(oper_string &str)
{
  Serial.println("~~~~~~~ OPER_STRING START ~~~~~~~~~~~");
  Serial.print("state:\t\t\t");
  Serial.println(str.state);

  Serial.print("reason:\t\t\t");
  Serial.println(INPUTS_ORIGIN[str.reason]);

  Serial.print("step:\t\t\t");
  Serial.println(str.step);

  Serial.print("ontime:\t\t\t");
  Serial.println(str.ontime);

  Serial.print("offtime:\t\t");
  Serial.println(str.offtime);
  Serial.println("~~~~~~~ OPER_STRING END ~~~~~~~~~~~~");
}

// ~~~~ All CBs are called eventually by timeoutButton instance OR external input (that call timeoutButton) ~~
void OFF_CB(uint8_t reason, uint8_t i)
{
  notifyOFF(reason, i);       /* First to display time elapsed */
  lightOutputV[i]->turnOFF(); /* and then turn off */
  update_OperString(reason, i, false);
}
void ON_CB(uint8_t reason, uint8_t i)
{
  if (lightOutputV[i]->turnON(lightOutputV[i]->currentStep)) /* First to display power */
  {
    notifyON(reason, i);
    update_OperString(reason, i, true);
  }
}
void MULTP_CB(uint8_t reason, uint8_t i)
{
  Serial.println(reason);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ External trigger (not physical button, but MQTT cmd) for timeout and light ~~~~~~~~~~~~~
void Ext_trigger_ON(uint8_t reason, int TO, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
    if (step != 0)
    {
      lightOutputV[i]->currentStep = step; /* Relevant for PWM lights */
    }
    timeoutButtonV[i]->ON_cb(TO, reason); /* emulates "button press" and at end calls ON_CB */
  }
}
void Ext_trigger_OFF(uint8_t reason, uint8_t i)
{
  if (i < numSW)
  {
    timeoutButtonV[i]->OFF_cb(reason);
  }
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
    if (lightOutputV[i]->isPWM())
    {
      if (timeoutButtonV[i]->getState()) /* if already ON */
      {
        if (lightOutputV[i]->turnON(step)) /* update PWM value */
        {
          notifyUpdatePWM(step, reason, i);
        }
      }
      else
      {
        Ext_trigger_ON(reason, timeoutButtonV[i]->defaultTimeout, step, i); /* if Off, turn ON with desired PWM value */
      }
      update_OperString(reason, i, true);
    }
  }
}
void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i)
{
  if (i < numSW)
  {
    if (timeoutButtonV[i]->getState())
    {
      notifyAdd(timeAdd, reason, i);
    }
    timeoutButtonV[i]->addWatch(timeAdd, reason); /* update end time */
    update_OperString(reason, i, true);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void onAtBoot(uint8_t i)
{
  if (OnatBoot[i])
  {
    Ext_trigger_ON(3, i);
  }
}
void checkResidue_onBoot(uint8_t i)
{
  read_OperStr(*OPERstringV[i]);
  Serial.print("Read OPERstring: \t#");
  Serial.println(i);
  print_OPERstring(*OPERstringV[i]);

  if (OPERstringV[i]->state == true && (iot.now() < OPERstringV[i]->offtime))
  {
    Serial.print("remained: ");
    Serial.println(OPERstringV[i]->offtime - iot.now());
    Serial.println((int)(OPERstringV[i]->offtime - iot.now()) / 60);
    Ext_trigger_ON(4, (int)(OPERstringV[i]->offtime - iot.now()) / 60, OPERstringV[i]->step);
  }
}

void init_TObutton(uint8_t i)
{
  timeoutButtonV[i]->ExtON_cb(ON_CB);
  timeoutButtonV[i]->ExtOFF_cb(OFF_CB);
  timeoutButtonV[i]->ExtMultiPress_cb(MULTP_CB);
  if (useInput[i])
  {
    timeoutButtonV[i]->begin(inputPin[i], trigType[i], i);
  }
  else
  {
    timeoutButtonV[i]->begin(i);
  }
}
void init_Light(uint8_t i)
{
  if (outputPWM[i])
  {
    lightOutputV[i]->init(outputPin[i], PWM_res, dimmablePWM[i]);
    lightOutputV[i]->defStep = defPWM[i];
    lightOutputV[i]->maxSteps = max_pCount[i];
    lightOutputV[i]->limitPWM = limitPWM[i];
  }
  else
  {
    lightOutputV[i]->init(outputPin[i], output_ON[i]);
  }
  if (useIndicLED[i])
  {
    lightOutputV[i]->auxFlag(indicPin[i]);
  }
}
void initAll()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    init_TObutton(i);
    init_Light(i);
    checkResidue_onBoot(i);
    onAtBoot(i);
  }
}
void debug_end_startUp()
{
  char a[130];
  Serial.println(F("\n ~~~~~~~~~~~~~~~~~~~~ POST-BOOT DEBUG START~~~~~~~~~~~~~~~~~~~~"));
  if (P_readOK_a && P_readOK_b && P_readOK_c)
  {
    Serial.println("All flash Paramters loaded OK");
  }
  else
  {
    sprintf(a, ">>> Error reading flash paramters:\nIOT2 vars loaded \t[%s]\nTopics loaded \t[%s]\nSketch vars loaded \t[%s]", P_readOK_a ? "OK" : "FAIL", P_readOK_b ? "OK" : "FAIL", P_readOK_b ? "OK" : "FAIL");
    Serial.print(a);
  }
  for (uint8_t i = 0; i < numSW; i++)
  {
    Serial.print("±±±±±± #");
    Serial.print(i);
    Serial.println(" ±±±±±± ");
    Serial.print("OnatBoot: \t");
    Serial.println(OnatBoot[i]);
    Serial.print("useInput: \t");
    Serial.println(useInput[i]);
    Serial.print("outputPWM: \t");
    Serial.println(outputPWM[i]);
    Serial.print("useIndicLED: \t");
    Serial.println(useIndicLED[i]);
    Serial.print("dimmablePWM: \t");
    Serial.println(dimmablePWM[i]);
    Serial.print("output_ON: \t");
    Serial.println(dimmablePWM[i]);
    Serial.print("inputPressed: \t");
    Serial.println(inputPressed[i]);
    Serial.print("trigType: \t");
    Serial.println(trigType[i]);
    Serial.print("inputPin: \t");
    Serial.println(inputPin[i]);
    Serial.print("outputPin: \t");
    Serial.println(outputPin[i]);
    Serial.print("indicPin: \t");
    Serial.println(indicPin[i]);
    Serial.print("def_TO_minutes:\t");
    Serial.println(def_TO_minutes[i]);
    Serial.print("maxON_minutes: \t");
    Serial.println(maxON_minutes[i]);
    Serial.print("defPWM: \t");
    Serial.println(defPWM[i]);
    Serial.print("max_pCount: \t");
    Serial.println(max_pCount[i]);
    Serial.print("limitPWM: \t");
    Serial.println(limitPWM[i]);
    Serial.print("sw_names: \t");
    Serial.println(sw_names[i]);
  }
  Serial.println(F(" ~~~~~~~~~~~~~~~~~~~~ POST-BOOT DEBUG END ~~~~~~~~~~~~~~~~~~~~\n"));
}

void setup()
{
  startIOTservices();
  initAll();
  debug_end_startUp();
}
void loop()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    timeoutButtonV[i]->loop();
  }
  iot.looper();
  delay(200);
}
