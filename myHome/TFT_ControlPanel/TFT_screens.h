#include <TFT_GUI.h>

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

extern void getIP(char *IPadd);
extern void update_topTitle(char *msg);
extern void rebuild_screen(bool a = true, bool b = true);
extern void clkUpdate(MessageTFT &txtBox);

extern void windowButtons_cb(uint8_t i);
extern void clearScreen(int c = 0);
extern void clearScreen(const uint16_t);

MessageTFT topTitle;
MessageTFT notePad;
ButtonTFT homeButton;
ButtonTFT backButton;
keypadTFT keypadAlarm;
buttonArrayTFT<4> startScreen;
buttonArrayTFT<4> mainWindows;
buttonArrayTFT<3> operateWindows;
buttonArrayTFT<8> specificWindows;
buttonArrayTFT<3> mainAlarm;
buttonArrayTFT<4> mainLights;
buttonArrayTFT<3> extLights;
buttonArrayTFT<3> roomLights;

bool use_homeButton = true;
bool use_backButton = true;
bool use_resetButton = true;

uint8_t menus_id = 0;
uint8_t button_id = 0;
uint8_t last_button_pressed = 0;
const uint8_t mainWindows_id = 10;
const uint8_t specificWindows_id = 20;
const uint8_t operWindows_id = 30;
const uint8_t mainAlarm_id = 40;
const uint8_t keypadAlarm_id = 50;
const uint8_t mainLights_id = 60;
const uint8_t extLights_id = 70;
const uint8_t roomsLights_id = 80;
const uint8_t kidsroomLights_id = 90;
const uint8_t parentsroomLights_id = 100;
const uint8_t familyroomLights_id = 110;
const uint8_t notePad_id = 120;
// const uint8_t resetButton_id = 130;
const uint8_t startScreen_id = 200;

const uint16_t TXT_COLOR = ILI9341_WHITE;
const uint16_t FACE_COLOR = ILI9341_DARKGREY;
const uint16_t BACKGROUND_COLOR = ILI9341_BLACK;

void create_mainAlarm()
{
    rebuild_screen();
    menus_id = mainAlarm_id;
    char *a[] = {"Arm Home", "Arm Away", "Disarm"};

    mainAlarm.scale_y = 72;
    mainAlarm.shift_y = 35;
    mainAlarm.txt_color = TXT_COLOR;
    mainAlarm.face_color = FACE_COLOR;
    mainAlarm.border_color = mainAlarm.face_color;
    mainAlarm.create_array(3, 1, a);
}
void create_keypadAlarm()
{
    menus_id = keypadAlarm_id;
    keypadAlarm.scale_f = 72;
    keypadAlarm.shift_y = 35;
    keypadAlarm.shift_x = 30;
    keypadAlarm.txt_color = TXT_COLOR;
    keypadAlarm.face_color = FACE_COLOR;
    keypadAlarm.border_color = keypadAlarm.face_color;

    keypadAlarm.create_keypad();
}

void create_mainWindows()
{
    rebuild_screen();
    menus_id = mainWindows_id;
    button_id = 0;
    char *a[] = {"All", "Saloon", "Room", "Specific"};

    mainWindows.scale_y = 72;
    mainWindows.shift_y = 35;
    mainWindows.txt_color = TXT_COLOR;
    mainWindows.face_color = FACE_COLOR;
    mainWindows.border_color = mainWindows.face_color;
    mainWindows.create_array(2, 2, a);
}
void create_specificWindows()
{
    rebuild_screen();
    menus_id = specificWindows_id;
    char *specificTitle[] = {"Family", "Parents", "Kids", "Single", "Dual", "Exit", "Laundry", "X"};
    specificWindows.scale_y = 72;
    specificWindows.shift_y = 35;
    specificWindows.txt_color = TXT_COLOR;
    specificWindows.face_color = FACE_COLOR;
    specificWindows.border_color = specificWindows.face_color;
    specificWindows.create_array(4, 2, specificTitle);
}
void create_operWindows()
{
    rebuild_screen();
    menus_id = operWindows_id;
    char *operTitle[] = {"Up", "Off", "Down"};
    operateWindows.scale_y = 72;
    operateWindows.shift_y = 35;
    operateWindows.txt_color = TXT_COLOR;
    operateWindows.face_color = FACE_COLOR;
    operateWindows.border_color = operateWindows.face_color;
    operateWindows.create_array(3, 1, operTitle);
}

void create_mainLights()
{
    menus_id = mainLights_id;
    char *a[] = {"External", "Internal", "Rooms", "Specific"};

    mainLights.scale_y = 72;
    mainLights.shift_y = 35;
    mainLights.txt_color = TXT_COLOR;
    mainLights.face_color = FACE_COLOR;
    mainLights.border_color = mainLights.face_color;
    mainLights.create_array(2, 2, a);
}
void create_roomsLights()
{
    menus_id = roomsLights_id;
    char *a[] = {"fam_Leds", "Par.Bed", "Par.Mirror", "Oz Bed", "Shac.Bed", "Shac.Mirror"};

    roomLights.scale_y = 72;
    roomLights.shift_y = 35;
    roomLights.txt_color = TXT_COLOR;
    roomLights.face_color = FACE_COLOR;
    roomLights.border_color = roomLights.face_color;
    roomLights.create_array(3, 2, a);
}
void create_extLights()
{
    menus_id = mainLights_id;
    char *a[] = {"fr. Door", "per.LEDs", "per. Bulb", "Specific"};

    extLights.scale_y = 72;
    extLights.shift_y = 35;
    extLights.txt_color = TXT_COLOR;
    extLights.face_color = FACE_COLOR;
    extLights.border_color = extLights.face_color;
    extLights.create_array(2, 2, a);
}

void create_startScreen()
{
    rebuild_screen(false, false);
    menus_id = startScreen_id;
    char *a[] = {"Alarm", "Windows", "Lights", "Setup"};

    startScreen.scale_y = 90;
    startScreen.shift_y = 35;
    startScreen.txt_color = TXT_COLOR;
    startScreen.face_color = FACE_COLOR;
    startScreen.border_color = startScreen.face_color;
    startScreen.roundRect = false;
    startScreen.create_array(2, 2, a);
}
void create_topTitle()
{
    topTitle.a = tft.width();
    topTitle.b = 30;
    topTitle.xc = tft.width() / 2;
    topTitle.yc = topTitle.b / 2;

    topTitle.txt_size = 2;
    topTitle.roundRect = false;
    topTitle.txt_color = ILI9341_WHITE;
    topTitle.face_color = ILI9341_BLUE;
    topTitle.border_color = topTitle.face_color;
    topTitle.createMSG("<< wait... >>");
}
void create_homeButton()
{
    homeButton.a = tft.width() / 2 - 5;
    homeButton.b = 40;
    homeButton.xc = tft.width() - homeButton.a / 2;
    homeButton.yc = tft.height() - homeButton.b / 2;

    homeButton.txt_size = 2;
    homeButton.roundRect = true;
    homeButton.txt_color = ILI9341_WHITE;
    homeButton.face_color = ILI9341_GREEN;
    homeButton.border_color = homeButton.face_color;
    homeButton.createButton(">>Home<<");
}
void create_backButton()
{
    backButton.a = tft.width() / 2 - 5;
    backButton.b = 40;
    backButton.xc = backButton.a / 2;
    backButton.yc = tft.height() - backButton.b / 2;

    backButton.txt_size = 2;
    backButton.roundRect = true;
    backButton.txt_color = ILI9341_WHITE;
    backButton.face_color = ILI9341_RED;
    backButton.border_color = backButton.face_color;
    backButton.createButton("<< Back");
}
void create_resetButton()
{
    use_resetButton = true;
    backButton.a = tft.width() / 2 - 5;
    backButton.b = 40;
    backButton.xc = backButton.a / 2;
    backButton.yc = tft.height() - backButton.b / 2;

    backButton.txt_size = 2;
    backButton.roundRect = true;
    backButton.txt_color = ILI9341_WHITE;
    backButton.face_color = ILI9341_RED;
    backButton.border_color = backButton.face_color;
    backButton.createButton("RESET");
}

void create_notePad()
{
    rebuild_screen(true, false);
    create_resetButton();

    menus_id = notePad_id;
    notePad.a = tft.width();
    notePad.b = 80;
    notePad.xc = notePad.a / 2;
    notePad.yc = 70;

    notePad.txt_size = 2;
    notePad.roundRect = false;
    notePad.txt_color = ILI9341_WHITE;
    notePad.face_color = FACE_COLOR;
    notePad.border_color = notePad.face_color;
    char ipadd[16];
    char uptime_clk[20];
    char uptime_day[8];
    char lines[3][30];

    iot.convert_epoch2clock(millis() / 1000, 0, uptime_clk, uptime_day);
    getIP(ipadd);
    sprintf(lines[0], "IP: %s", ipadd);
    sprintf(lines[1], "upTime: %s %s", uptime_day, uptime_clk);
    sprintf(lines[2], "MQTT: %s", iot.deviceTopic);
    char *noteLines[] = {lines[0], lines[1], lines[2]};
    notePad.createPage(noteLines, 3);
}

void windows_button_looper(TS_Point &p)
{
    if (menus_id == mainWindows_id)
    {
        uint8_t i = mainWindows.checkPress(p);
        if (i != 99)
        {
            if (i != 3)
            {
                button_id = mainWindows_id + i;
                create_operWindows();
            }
            else
            {
                create_specificWindows();
            }
        }
    }
    else if (menus_id == specificWindows_id)
    {
        uint8_t i = specificWindows.checkPress(p);
        if (i != 99)
        {
            button_id = specificWindows_id + i;
            create_operWindows();
        }
    }
    else if (menus_id == operWindows_id)
    {
        uint8_t i = operateWindows.checkPress(p);
        windowButtons_cb(i);
    }
}
void Lights_looper(TS_Point &p)
{
    if (menus_id == mainLights_id)
    {
        uint8_t i = mainLights.checkPress(p);
        if (i != 99)
        {
            if (i == 0)
            {
                rebuild_screen();
                create_extLights();
            }
            else if (i == 1)
            {
                rebuild_screen();
            }
            else if (i == 2)
            {
                rebuild_screen();
                create_roomsLights();
            }
        }
    }
}
bool homeButton_looper(TS_Point &p)
{
    if (use_homeButton && homeButton.checkPress(p))
    {
        create_startScreen();
        return true;
    }
    else
    {
        return false;
    }
}
bool backButton_looper(TS_Point &p)
{
    if (use_backButton && backButton.checkPress(p))
    {
        if (menus_id == mainAlarm_id || menus_id == mainLights_id || menus_id == mainWindows_id || menus_id == notePad_id)
        {
            create_startScreen();
            return true;
        }
        else if (menus_id == specificWindows_id)
        {
            create_mainWindows();
            return true;
        }
        else if (menus_id == operWindows_id)
        {
            if (button_id > operWindows_id)
            {
                create_specificWindows();
                return true;
            }
            else
            {
                create_mainWindows();
                return true;
            }
        }
        else if (menus_id == keypadAlarm_id)
        {
            create_mainAlarm();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
void resetButton_looper(TS_Point &p)
{
    if (backButton.checkPress(p))
    {
        iot.sendReset("TFT_Button");
    }
}
void topTitle_looper()
{
    static bool enter_once = true;
    if (menus_id == startScreen_id)
    {
        clkUpdate(topTitle);
    }
    else if (menus_id == keypadAlarm_id && enter_once == true)
    {
        enter_once = false;
        update_topTitle("Press # to send");
    }
}

void alarm_looper(TS_Point &p)
{
    uint8_t i = mainAlarm.checkPress(p);
    if (i != 99)
    {
        if (i == 2)
        {
            // clearScreen();
            rebuild_screen();
            create_keypadAlarm();
            Serial.println(i);
        }
        else if (i == 1)
        {
            iot.pub_noTopic("armed_home", "myHome/alarmMonitor");
            Serial.println(i);
        }
        else if (i == 0)
        {
            iot.pub_noTopic("armed_away", "myHome/alarmMonitor");
            Serial.println(i);
        }
    }
}
void alarmKeypad_looper(TS_Point &p)
{
    if (keypadAlarm.getPasscode(p))
    {
        if (strcmp("1234", keypadAlarm.keypad_value) == 0)
        {
            update_topTitle("PassCode OK");
            // iot.pub_noTopic("disarmed", "myHome/alarmMonitor");
            delay(1000);
            rebuild_screen();
            create_startScreen();
        }
        else
        {
            update_topTitle("PassCode Fail");
            delay(1000);
            create_keypadAlarm();
        }
    }
    else
    {
        char e[keypadAlarm.counter + 2];
        sprintf(e, "");
        for (uint8_t i = 0; i < keypadAlarm.counter; i++)
        {
            strcat(e, "*");
        }
        update_topTitle(e);
    }
}
void startScreen_looper(TS_Point &p)
{
    uint8_t i = startScreen.checkPress(p);
    if (i != 99)
    {
        if (i == 0)
        {
            create_mainAlarm();
        }
        else if (i == 1)
        {
            create_mainWindows();
        }
        else if (i == 2)
        {
            create_mainLights();
        }
        else if (i == 3)
        {
            create_notePad();
        }
    }
}
