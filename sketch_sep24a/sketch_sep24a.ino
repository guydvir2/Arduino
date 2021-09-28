#include <myIOT2.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 0

myIOT2 iot;
XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

ButtonTFT But_LIGHTS(ts, tft);
ButtonTFT But_WINDOWS(ts, tft);
ButtonTFT But_ALARN(ts, tft);

ButtonTFT But_NAV_back(ts, tft);
ButtonTFT But_NAV_home(ts, tft);

ButtonTFT But_Win_All(ts, tft);
ButtonTFT But_Win_Rooms(ts, tft);
ButtonTFT But_Saloon(ts, tft);
ButtonTFT But_Specific(ts, tft);
ButtonTFT But_Win_family(ts, tft);
ButtonTFT But_Win_parents(ts, tft);
ButtonTFT But_Win_kids(ts, tft);
ButtonTFT But_Win_exit(ts, tft);
ButtonTFT But_Win_single(ts, tft);
ButtonTFT But_Win_dual(ts, tft);

ButtonTFT But_Win_cmdUP(ts, tft);
ButtonTFT But_Win_cmdDOWN(ts, tft);
ButtonTFT But_Win_cmdSTOP(ts, tft);

ButtonTFT Alarm_home(ts, tft);
ButtonTFT Alarm_full(ts, tft);
ButtonTFT Alarm_disarmed(ts, tft);

MessageTFT MSG_WiFi(tft);

int window_t = 0;
int current_menu = 0;
bool conn_state = false;

void clearScreen()
{
  tft.fillScreen(ILI9341_YELLOW);
}
void start_GUI()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen();
}

void msg_wifi(bool &state)
{
  const uint8_t txt_size = 1;
  const uint8_t MSG_size_y = 15;
  const uint8_t MSG_size_x = 60;

  MSG_WiFi.screen_rotation = SCREEN_ROT;
  MSG_WiFi.txt_size = txt_size;
  MSG_WiFi.a = MSG_size_x;
  MSG_WiFi.b = MSG_size_y;
  MSG_WiFi.xc = MSG_size_x / 2;
  MSG_WiFi.yc = MSG_WiFi.b / 2;
  MSG_WiFi.border_thickness = 1;
  MSG_WiFi.roundRect = false;
  if (state)
  {
    MSG_WiFi.text("Wifi: OK");
    MSG_WiFi.face_color = ILI9341_GREEN;
  }
  else
  {
    MSG_WiFi.text("Wifi: No");
    MSG_WiFi.face_color = ILI9341_RED;
  }
  MSG_WiFi.drawMSG();
}

void menu_0()
{
  current_menu = 0;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 150;
  const uint8_t but_size_b = 75;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Lights", "Windows", "Alarm"};

  ButtonTFT *buttons[] = {&But_LIGHTS, &But_WINDOWS, &But_ALARN};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_1_1()
{
  current_menu = 11;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 150;
  const uint8_t but_size_b = 75;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Indoors", "Outdoors", "Rooms"};

  ButtonTFT *buttons[] = {&Alarm_home, &Alarm_full, &Alarm_disarmed};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_1_1_1()
{
  current_menu = 111;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 125;
  const uint8_t but_size_b = 60;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Stove", "Kitchen", "Saloon"};

  ButtonTFT *buttons[] = {&Alarm_home, &Alarm_full, &Alarm_disarmed};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_1_1_1()
{
  current_menu = 111;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 125;
  const uint8_t but_size_b = 60;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Stove", "Kitchen", "Saloon"};

  ButtonTFT *buttons[] = {&Alarm_home, &Alarm_full, &Alarm_disarmed};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_2_1() /* main windows */
{
  current_menu = 21;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 125;
  const uint8_t but_size_b = 60;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"All", "Rooms", "Saloon", "Specific"};

  ButtonTFT *buttons[] = {&But_Win_All, &But_Win_Rooms, &But_Saloon, &But_Specific};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_2_1_1() /* specific windows*/
{
  current_menu = 211;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 125;
  const uint8_t but_size_b = 60;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Family", "Parents", "Kids", "S.Exit", "S.Single", "S.Dual"};

  ButtonTFT *buttons[] = {&But_Win_family, &But_Win_parents, &But_Win_kids, &But_Win_exit, &But_Win_single, &But_Win_dual};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_2_1_1_1() /* Commands to MQTT Windows */
{
  current_menu = 2111;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 150;
  const uint8_t but_size_b = 75;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Up", "Stop", "Down"};

  ButtonTFT *buttons[] = {&But_Win_cmdUP, &But_Win_cmdSTOP, &But_Win_cmdDOWN};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_3_1() /* Alarm Main */
{
  current_menu = 31;
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 150;
  const uint8_t but_size_b = 75;
  const uint8_t but_space = 5;
  char *txt_buttons[] = {"Home", "Full", "Disarm"};

  ButtonTFT *buttons[] = {&Alarm_home, &Alarm_full, &Alarm_disarmed};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = tft.width() / 2;
    buttons[c]->yc = (c + 1) * (tft.height() / (num_items + 1));
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}
void menu_4() /* Nav Buttons */
{
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 60;
  const uint8_t but_size_b = 30;
  char *txt_buttons[] = {"Back", "Home"};
  ButtonTFT *buttons[] = {&But_NAV_back, &But_NAV_home};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    buttons[c]->screen_rotation = SCREEN_ROT;
    buttons[c]->text(txt_buttons[c]);
    buttons[c]->txt_size = txt_size;
    buttons[c]->a = but_size_a;
    buttons[c]->b = but_size_b;
    buttons[c]->xc = but_size_a / 2 + c * (tft.width() - but_size_a);
    buttons[c]->yc = tft.height() - but_size_b / 2;
    buttons[c]->face_color = ILI9341_RED;
    buttons[c]->border_color = ILI9341_GREEN;
    buttons[c]->roundRect = true;
    buttons[c]->latchButton = false;
    buttons[c]->drawButton();
  }
}

void loop_menu_0(TS_Point &p)
{
  ButtonTFT *buttons[] = {&But_LIGHTS, &But_WINDOWS, &But_ALARN};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 1)
      {
        clearScreen();
        menu_4();
        menu_2_1();
      }
      else if (i == 2)
      {
        clearScreen();
        menu_4();
        menu_3_1();
      }
      Serial.println(buttons[i]->txt_buf);
    }
  }
}
void loop_menu_2_1(TS_Point &p)
{
  ButtonTFT *buttons[] = {&But_Win_All, &But_Win_Rooms, &But_Saloon, &But_Specific};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 3)
      {
        clearScreen();
        menu_4();
        menu_2_1_1();
        Serial.println("Specific");
      }
      else
      {
        clearScreen();
        menu_4();
        menu_2_1_1_1();
      }
      Serial.println(buttons[i]->txt_buf);
      window_t = i;
    }
  }
}
void loop_menu_2_1_1(TS_Point &p)
{
  ButtonTFT *buttons[] = {&But_Win_family, &But_Win_parents, &But_Win_kids, &But_Win_exit};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      clearScreen();
      menu_4();
      menu_2_1_1_1();
      Serial.println(buttons[i]->txt_buf);
      window_t = 10 + i;
    }
  }
}
void loop_menu_2_1_1_1(TS_Point &p)
{
  ButtonTFT *buttons[] = {&But_Win_cmdUP, &But_Win_cmdSTOP, &But_Win_cmdDOWN};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);
  char *baseTopic = "myHome/Windows";   /* MQTT Topic prefix*/
  char *cmds[] = {"up", "off", "down"}; /* MQTT CMD*/
  char msg[40];
  char groups[] = {};

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (window_t == 0) /* All */
      {
        iot.pub_noTopic(cmds[i], baseTopic);
      }
      else if (window_t == 1) /* All Rooms */
      {
        char *roomTopics[] = {"familyRoom", "parentsRoom", "kidsRoom"};
        for (uint8_t n = 0; n < sizeof(roomTopics) / sizeof(roomTopics[0]); n++)
        {
          sprintf(msg, "%s/%s", baseTopic, roomTopics[n]);
          iot.pub_noTopic(cmds[i], msg);
          Serial.println(msg);
        }
      }
      else if (window_t == 2) /* All SaloonRooms */
      {
        char *roomTopics[] = {"saloonSingle", "saloonExit", "saloonDual"};
        for (uint8_t n = 0; n < sizeof(roomTopics) / sizeof(roomTopics[0]); n++)
        {
          sprintf(msg, "%s/%s", baseTopic, roomTopics[n]);
          iot.pub_noTopic(cmds[i], msg);
        }
      }
      else if (window_t >= 10) /* Specific Windows */
      {
        char *roomTopics[] = {"familyRoom", "parentsRoom", "kidsRoom", "saloonExit"};
        sprintf(msg, "%s/%s", baseTopic, roomTopics[window_t - 10]);
        iot.pub_noTopic(cmds[i], msg);
      }
      Serial.println(buttons[i]->txt_buf);
    }
  }
}
void loop_menu_3_1(TS_Point &p)
{
  ButtonTFT *buttons[] = {&Alarm_home, &Alarm_full, &Alarm_disarmed};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      char *topic = "myHome/alarmMonitor";
      char *cmds[] = {"armed_home", "armed_away", "disarmed"};
      iot.pub_noTopic(cmds[i], topic);
      clearScreen();
      menu_4();
      Serial.println(buttons[i]->txt_buf);
    }
  }
}
void loop_menu_4(TS_Point &p)
{
  ButtonTFT *buttons[] = {&But_NAV_back, &But_NAV_home};
  uint8_t num_items = sizeof(buttons) / sizeof(buttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 1)
      {
        clearScreen();
        menu_4();
        menu_0();
      }
      else
      {
      }
      Serial.println(buttons[i]->txt_buf);
    }
  }
}

#define DEV_TOPIC "tft"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"
void check_wifi()
{
  static unsigned long last_check = 0;
  if (millis() - last_check > 30000)
  {
    last_check = millis();
    conn_state = true;
  }
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "BOOOOO");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
    // iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help2: Commands #2 - [; m; ,x]");
    iot.pub_msg(msg);
  }
}
void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = true;
  iot.useextTopic = false;
  iot.useDebug = true;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
  iot.noNetwork_reset = 10;
  iot.useBootClockLog = true;
  strcpy(iot.deviceTopic, DEV_TOPIC);
  strcpy(iot.prefixTopic, PREFIX_TOPIC);
  strcpy(iot.addGroupTopic, GROUP_TOPIC);
  iot.start_services(addiotnalMQTT);
}

void setup()
{
  start_GUI();
  menu_0();
  menu_4();
  startIOTservices();
}
void loop()
{
  static unsigned long last_touch = 0;
  if (ts.touched() && millis() - last_touch > 500)
  {
    last_touch = millis();
    TS_Point p = ts.getPoint();
    loop_menu_4(p); /* Always appears */
    if (current_menu == 0)
    {
      loop_menu_0(p);
    }
    else if (current_menu == 21)
    {
      loop_menu_2_1(p);
    }
    else if (current_menu == 211)
    {
      loop_menu_2_1_1(p);
    }
    else if (current_menu == 2111)
    {
      loop_menu_2_1_1_1(p);
    }
    else if (current_menu == 31)
    {
      loop_menu_3_1(p);
    }
  }
  msg_wifi(conn_state);
  iot.looper();
}
