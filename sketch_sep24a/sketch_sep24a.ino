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

#define DEV_TOPIC "tft"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"

#define SCREEN_ROT 0

myIOT2 iot;
XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

ButtonTFT But_NAV_back(ts, tft);
ButtonTFT But_NAV_home(ts, tft);
ButtonTFT generic_but_0(ts, tft);
ButtonTFT generic_but_1(ts, tft);
ButtonTFT generic_but_2(ts, tft);
ButtonTFT generic_but_3(ts, tft);
ButtonTFT generic_but_4(ts, tft);
ButtonTFT generic_but_5(ts, tft);
ButtonTFT generic_but_6(ts, tft);
ButtonTFT generic_but_7(ts, tft);
ButtonTFT generic_but_8(ts, tft);
ButtonTFT generic_but_9(ts, tft);
ButtonTFT generic_but_10(ts, tft);
ButtonTFT generic_but_11(ts, tft);
MessageTFT MSG_WiFi(tft);

ButtonTFT *NAVbuttons[] = {&But_NAV_back, &But_NAV_home};
ButtonTFT *buttons[] = {&generic_but_0, &generic_but_1, &generic_but_2, &generic_but_3, &generic_but_4, &generic_but_5,
                        &generic_but_6, &generic_but_7, &generic_but_8, &generic_but_9, &generic_but_10, &generic_but_11};

int window_t = 0;
int current_menu = 0;

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

void msg_wifi(bool state)
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

void create_buttons(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size = 2)
{
  const uint8_t but_space = 5;
  const uint8_t but_size_a = (uint8_t)((tft.width() - 50) / C);
  const uint8_t but_size_b = (uint8_t)((tft.height() - 50) / R);
  const uint8_t x_margin = (int)(tft.width() + (1 - C) * (but_size_a + but_space)) / 2;
  const uint8_t y_margin = (int)(tft.height() + (1 - R) * (but_size_b + but_space)) / 2;

  for (uint8_t r = 0; r < R; r++)
  {
    for (uint8_t c = 0; c < C; c++)
    {
      buttons[C * r + c]->screen_rotation = SCREEN_ROT;
      buttons[C * r + c]->text(but_txt[C * r + c]);
      buttons[C * r + c]->txt_size = txt_size;
      buttons[C * r + c]->a = but_size_a;
      buttons[C * r + c]->b = but_size_b;
      buttons[C * r + c]->xc = x_margin + c * (but_size_a + but_space);
      buttons[C * r + c]->yc = y_margin + r * (but_size_b + but_space);
      buttons[C * r + c]->roundRect = true;
      buttons[C * r + c]->latchButton = false;
      buttons[C * r + c]->drawButton();
    }
  }
}
void menu_0()
{
  current_menu = 0;
  char *txt_buttons[] = {"Lights", "Windows", "Alarm"};
  create_buttons(3, 1, txt_buttons);
}
void menu_1_1() /* Lights main*/
{
  current_menu = 11;
  char *txt_buttons[] = {"Indoors", "Outdoors", "Rooms"};
  create_buttons(3, 1, txt_buttons);
}
void menu_1_1_1() /* Indoor Lights */
{
  current_menu = 111;
  char *txt_buttons[] = {"Stove", "Kitchen", "Saloon"};
  create_buttons(3, 1, txt_buttons);
}
void menu_1_1_2() /* outDoor Lights */
{
  current_menu = 112;
  char *txt_buttons[] = {"A", "B", "C", "D", "E", "F"};
  create_buttons(3, 2, txt_buttons);
}
void menu_1_1_3() /* Room Lights */
{
  current_menu = 113;
  char *txt_buttons[] = {"F.R LEDs", "P.R Mirror", "P.R Bed", "K.R Mirror", "K.R Bed_1", "K.R Bed_2"};
  create_buttons(3, 2, txt_buttons, 1);
}

void menu_2_1() /* main windows */
{
  current_menu = 21;
  char *txt_buttons[] = {"All", "Rooms", "Saloon", "Specific"};
  create_buttons(4, 1, txt_buttons);
}
void menu_2_1_1() /* specific windows*/
{
  current_menu = 211;
  char *txt_buttons[] = {"Family", "Parents", "Kids", "S.Exit", "S.Single", "S.Dual"};
  create_buttons(3, 2, txt_buttons);
}
void menu_2_1_1_1() /* Commands to MQTT Windows */
{
  current_menu = 2111;
  char *txt_buttons[] = {"Up", "Stop", "Down"};
  create_buttons(3, 1, txt_buttons);
}
void menu_3_1() /* Alarm Main */
{
  current_menu = 31;
  char *txt_buttons[] = {"Home", "Full", "Disarm"};
  create_buttons(3, 1, txt_buttons);
}
void menu_4() /* Nav Buttons */
{
  const uint8_t txt_size = 2;
  const uint8_t but_size_a = 60;
  const uint8_t but_size_b = 30;
  char *txt_buttons[] = {"Back", "Home"};
  uint8_t num_items = sizeof(txt_buttons) / sizeof(txt_buttons[0]);

  for (uint8_t c = 0; c < num_items; c++)
  {
    NAVbuttons[c]->screen_rotation = SCREEN_ROT;
    NAVbuttons[c]->text(txt_buttons[c]);
    NAVbuttons[c]->txt_size = txt_size;
    NAVbuttons[c]->a = but_size_a;
    NAVbuttons[c]->b = but_size_b;
    NAVbuttons[c]->xc = but_size_a / 2 + c * (tft.width() - but_size_a);
    NAVbuttons[c]->yc = tft.height() - but_size_b / 2;
    NAVbuttons[c]->face_color = ILI9341_RED;
    NAVbuttons[c]->border_color = ILI9341_GREEN;
    NAVbuttons[c]->roundRect = true;
    NAVbuttons[c]->latchButton = false;
    NAVbuttons[c]->drawButton();
  }
}

void loop_menu_0(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 0)
      {
        clearScreen();
        //menu_4();
        menu_1_1();
      }
      else if (i == 1)
      {
        clearScreen();
        //menu_4();
        menu_2_1();
      }
      else if (i == 2)
      {
        clearScreen();
        //menu_4();
        menu_3_1();
      }
    }
  }
}
void loop_menu_1_1(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      Serial.println(buttons[i]->txt_buf);
      if (i == 0)
      {
        clearScreen();
        menu_1_1_1();
        //menu_4();
      }
      else if (i == 1)
      {
        clearScreen();
        menu_1_1_2();
        //menu_4();
      }
      else if (i == 2)
      {
        clearScreen();
        menu_1_1_3();
        //menu_4();
      }
    }
  }
}
void loop_menu_1_1_1(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      Serial.println(buttons[i]->txt_buf);
      if (i == 0)
      {
        clearScreen();
        menu_0();
      }
      else if (i == 1)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
      else if (i == 2)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
    }
  }
}
void loop_menu_1_1_2(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      Serial.println(buttons[i]->txt_buf);
      if (i == 0)
      {
        clearScreen();
        menu_0();
      }
      else if (i == 1)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
      else if (i == 2)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
      else
      {
        clearScreen();
        menu_0();
      }
    }
  }
}
void loop_menu_1_1_3(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      Serial.println(buttons[i]->txt_buf);
      if (i == 0)
      {
        clearScreen();
        menu_0();
      }
      else if (i == 1)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
      else if (i == 2)
      {
        clearScreen();
        menu_0();
        //menu_4();
      }
      else
      {
        clearScreen();
        menu_0();
      }
    }
  }
}
void loop_menu_2_1(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 3)
      {
        clearScreen();
        //menu_4();
        menu_2_1_1();
      }
      else
      {
        clearScreen();
        //menu_4();
        menu_2_1_1_1();
      }
      Serial.println(buttons[i]->txt_buf);
      window_t = i;
    }
  }
}
void loop_menu_2_1_1(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      clearScreen();
      //menu_4();
      menu_2_1_1_1();
      Serial.println(buttons[i]->txt_buf);
      window_t = 10 + i;
    }
  }
}
void loop_menu_2_1_1_1(TS_Point &p, uint8_t num_items)
{
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
void loop_menu_3_1(TS_Point &p, uint8_t num_items)
{
  char *topic = "myHome/alarmMonitor";
  char *cmds[] = {"armed_home", "armed_away", "disarmed"};

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      iot.pub_noTopic(cmds[i], topic);
      Serial.println(buttons[i]->txt_buf);
    }
  }
}
void loop_menu_4(TS_Point &p)
{
  uint8_t num_items = sizeof(NAVbuttons) / sizeof(NAVbuttons[0]);

  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      if (i == 1)
      {
        clearScreen();
        //menu_4();
        menu_0();
      }
      else
      {
      }
      Serial.println(buttons[i]->txt_buf);
    }
  }
}

void check_wifi()
{
  static unsigned long last_check = 0;
  if (millis() - last_check > 5000)
  {
    last_check = millis();
    if (WiFi.status() == WL_CONNECTED)
    {
      msg_wifi(true);
    }
    else
    {
      msg_wifi(true);
    }
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
  // //menu_4();
  startIOTservices();
}
void loop()
{
  static unsigned long last_touch = 0;
  // Serial.println(current_menu);
  if (millis() - last_touch > 500)
  {
    if (ts.touched())
    {
      // if (millis() - last_touch > 500 && millis() - last_touch < 5000 || last_touch == 0)
      // {
      last_touch = millis();
      TS_Point p = ts.getPoint();
      // loop_//menu_4(p); /* Always appear */
      if (current_menu == 0)
      {
        loop_menu_0(p, 3);
      }
      else if (current_menu == 11)
      {
        loop_menu_1_1(p, 3);
        Serial.println("a");
      }
      else if (current_menu == 21)
      {
        loop_menu_2_1(p, 4);
      }
      else if (current_menu == 31)
      {
        loop_menu_3_1(p, 3);
      }
      else if (current_menu == 111)
      {
        Serial.println("b");
        loop_menu_1_1_1(p, 3);
      }
      else if (current_menu == 112)
      {
        Serial.println("c");
        loop_menu_1_1_2(p, 6);
      }
      else if (current_menu == 113)
      {
        Serial.println("d");
        loop_menu_1_1_3(p, 6);
      }

      else if (current_menu == 211)
      {
        loop_menu_2_1_1(p, 6);
      }
      else if (current_menu == 2111)
      {
        loop_menu_2_1_1_1(p, 3);
      }

      else
      {
        Serial.println("Nothing");
      }
    }
  }
  // }
  // else
  // {
  //   if (millis() - last_touch > 5000 && current_menu != 0)
  //   {
  //     clearScreen();
  //     menu_0();
  //     //menu_4();
  //     last_touch = 0;
  //   }
  // }
  // check_wifi();
  iot.looper();
}
