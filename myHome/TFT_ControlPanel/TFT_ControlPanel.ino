#include "myIOT_settings.h"
#include "TFT_screens.h"
#define SCREEN_ROT 0

void clearScreen(int c)
{
  if (c == 0)
  {
    tft.fillScreen(ILI9341_BLACK);
  }
  else if (c == 1)
  {
    tft.fillScreen(ILI9341_YELLOW);
  }
  else if (c == 2)
  {
    tft.fillScreen(ILI9341_BLUE);
  }
}
void clearScreen(const uint16_t c)
{
  tft.fillScreen(c);
}
void start_touchScreen()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
}

void windowButtons_cb(uint8_t i)
{
  char *top = "myHome/Windows";
  char *cmds[] = {"up", "off", "down"};
  char *specif[] = {"familyRoom", "parentsRoom", "kidsRoom", "saloonSingle",
                    "saloonDual", "saloonExit", "Laundry", "X"};

  if (i != 99)
  {
    char fultop[30];
    if (button_id == mainWindows_id)
    {
      iot.pub_noTopic(cmds[i], top);
    }
    else if (button_id == mainWindows_id + 1) // Sallon Windows
    {
      for (uint8_t a = 3; a < 6; a++)
      {
        sprintf(fultop, "%s/%s", top, specif[a]);
        iot.pub_noTopic(cmds[i], fultop);
      }
    }
    else if (button_id == mainWindows_id + 2) // Room Windows
    {
      for (uint8_t a = 0; a < 3; a++)
      {
        sprintf(fultop, "%s/%s", top, specif[a]);
        iot.pub_noTopic(cmds[i], fultop);
      }
    }
    else if (button_id >= specificWindows_id && button_id < specificWindows_id + 7) // Room Windows
    {
      sprintf(fultop, "%s/%s", top, specif[button_id - specificWindows_id]);
      iot.pub_noTopic(cmds[i], fultop);
    }
    else
    {
      yield;
    }
  }
  else
  {
    yield;
  }
}
void roomLights_cb(uint8_t i, uint8_t state, uint8_t a)
{
  char *top = "myHome/intLights";
  char num_on[10];
  char num_off[10];
  sprintf(num_on, "%d,on", a);
  sprintf(num_off, "%d,off", a);

  char *cmds[] = {num_off, num_on};
  char *specif[] = {"familyRoomLEDs", "parentsClosetLEDs", "parentsClosetLEDs", "shacharCloset", "dotClock",
                    "parentsBedLEDs", "kidsBed", "kidsBed", "parentsBedLEDs", "toiletLEDs"};
  if (i != 99)
  {
    char fultop[50];
    sprintf(fultop, "%s/%s", top, specif[i]);
    if (i != 4)
    {
      iot.pub_noTopic(cmds[state], fultop);
    }
    else
    {
      char *cmd[] = {"off", "on"};
      iot.pub_noTopic(cmd[state], fultop);
    }
  }
}
void internalLights_cb(uint8_t i, uint8_t state)
{
  char *top = "myHome/intLights";
  char *cmds[] = {"0,off", "0,on"};
  char *specif[] = {"Stove", "KitchenLEDs", "LivingRoom"};
  if (i != 99)
  {
    char fultop[50];
    sprintf(fultop, "%s/%s", top, specif[i]);
    iot.pub_noTopic(cmds[state], fultop);
    Serial.print(fultop);
    Serial.println(cmds[state]);
  }
}

void update_topTitle(char *msg, const uint16_t c)
{
  topTitle.face_color = c;
  topTitle.createMSG(msg);
}
void getIP(char *IPadd)
{
  static unsigned long lastLoop = 0;
  static bool lastState = false;

  if (WiFi.isConnected())
  {
    sprintf(IPadd, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  }
}
void getClk(char *clk)
{
  iot.return_clock(clk);
}

void rebuild_screen(bool a, bool b)
{
  clearScreen(BACKGROUND_COLOR);
  create_topTitle();
  use_homeButton = a;
  use_backButton = b;
  if (a)
  {
    create_homeButton();
  }
  if (b)
  {
    create_backButton();
    use_resetButton = false;
  }
}
void clkUpdate(MessageTFT &txtBox)
{
  static bool wifi_constate = false;
  static unsigned long last_clkUpdate = 0;
  const uint16_t connected_faceColor = ILI9341_BLUE;
  const uint16_t disconnected_faceColor = ILI9341_RED;

  if (millis() - last_clkUpdate > 1000)
  {
    char a[20];
    iot.return_clock(a);
    txtBox.updateTXT(a);
    last_clkUpdate = millis();

    if (WiFi.isConnected() != wifi_constate)
    {
      wifi_constate = WiFi.isConnected();
      if (wifi_constate == false)
      {
        txtBox.face_color = disconnected_faceColor;
        txtBox.border_color = disconnected_faceColor;
        txtBox.createMSG(txtBox.txt_buf);
      }
      else
      {
        txtBox.face_color = connected_faceColor;
        txtBox.border_color = connected_faceColor;
        txtBox.createMSG(txtBox.txt_buf);
      }
    }
  }
}
void setup()
{
  start_touchScreen();
  startIOTservices();
  create_startScreen();
}
void loop()
{
  const uint8_t backHome_seconds = 60;
  static unsigned long lastPress_counter = 0;

  if (ts.touched())
  {
    lastPress_counter = millis();
    TS_Point p = ts.getPoint();
    {
      if (homeButton_looper(p))
      {
        return;
      }
      if (backButton_looper(p))
      {
        return;
      }
      if (resetButton_looper(p))
      {
        return;
      }

      if (menus_id == startScreen_id)
      {
        startScreen_looper(p);
      }
      else if (menus_id >= mainWindows_id && menus_id <= operWindows_id)
      {
        windows_button_looper(p);
      }
      else if (menus_id == mainAlarm_id)
      {
        alarm_looper(p);
      }
      else if (menus_id == keypadAlarm_id)
      {
        alarmKeypad_looper(p);
      }
      else if (menus_id == mainLights_id)
      {
        Lights_looper(p);
      }
      else if (menus_id == roomsLights_id)
      {
        roomLights_looper(p);
      }
      else if (menus_id == internalLights_id)
      {
        internalLights_looper(p);
      }

      delay(500);
    }
  }

  if (lastPress_counter != 0 && (millis() - lastPress_counter >= 1000 * backHome_seconds) && menus_id != startScreen_id)
  {
    lastPress_counter = 0;
    create_startScreen();
  }
  topTitle_looper();
  iot.looper();
}
