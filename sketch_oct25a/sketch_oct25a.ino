#include <ArduinoJson.h>
#include <FS.h>

#if defined(ESP32)
#include <LITTLEFS.h>
#define LITFS LITTLEFS
#elif defined(ESP8266)
#include <LittleFS.h>
#define LITFS LittleFS
#endif

enum SAVED_STATES : const uint8_t
{
  BOOT_OK,
  BOOT_INIT,
  BOOT_FAIL,
  BOOT_INSTABLE
};
enum ERR_TYPES : const uint8_t
{
  NO_ERR,
  SETUP_ERR,
  LOOP_ERR,
  UNDEF_ERR,
  READ_ERR
};
enum CONT_TYPES : const uint8_t
{
  CONT_OK,
  CONT_STOP,
  CONT_NOTSURE
};

bool _useSerial = true;
bool loopSave = false;
uint8_t cont_flag = CONT_NOTSURE;
const int DOC_SIZE = 128;
const char *_filename = "\boot.json";
const char *keys[] = {"BOOT_0", "BOOT_1", "BOOT_TEMP"};

bool startFS()
{
#if defined(ESP32)
  bool a = LITFS.begin(true);
#elif defined(ESP8266)
  bool a = LITFS.begin();
#endif
  return a;
}
bool _readFile(JsonDocument &_doc)
{
  File readFile = LITFS.open(_filename, "r");
  DeserializationError error = deserializeJson(_doc, readFile);
  readFile.close();
  if (error)
  {
    if (_useSerial)
    {
      Serial.println(F("Failed to read JSON file"));
      Serial.println(_filename);
      Serial.println(error.c_str());
    }
    return 0;
  }
  else
  {
    return 1;
  }
}
void _saveFile(JsonDocument &_doc)
{
  File writeFile = LITFS.open(_filename, "w");
  serializeJson(_doc, writeFile);
  writeFile.close();
}
bool getValue(const char *key, byte &retval)
{
  DynamicJsonDocument _tempJDOC(DOC_SIZE);
  _readFile(_tempJDOC);
  bool hasKey = _tempJDOC.containsKey(key);
  if (hasKey)
  {
    retval = _tempJDOC[key];
    _tempJDOC.clear();
    return 1;
  }
  else
  {
    _tempJDOC.clear();
    return 0; // when key is not present
  }
}
void setValue(const char *key, byte value)
{
  DynamicJsonDocument _tempJDOC(DOC_SIZE);
  _readFile(_tempJDOC);
  _tempJDOC[key] = value;
  _saveFile(_tempJDOC);
  _tempJDOC.clear();
}

uint8_t last_boot_state(uint8_t i = 0)
{
  // uint8_t _last = 0;
  // uint8_t boot_state[3]{};

  // for (uint8_t n = 0; n < i; n++)
  // {
  //   uint8_t a = read_prev_boot(0);
  //   _last = max(a, _last);
  // }
  uint8_t a = read_prev_boot(n);
  return a;
}
void loop_bootUpdate(uint8_t t = 2)
{
  if (millis() > t * 1000 && loopSave == false)
  {
    loopSave = true;
    if (startFS())
    {
      setValue(0, BOOT_OK);
      Serial.println("Late save OK");
    }
    else
    {
      Serial.println("FS - late save err");
    }
  }
}
byte read_prev_boot(uint8_t i = 0)
{
  byte retval{};
  if (getValue(keys[i], retval))
  {
    return retval;
  }
  else
  {
    return READ_ERR;
  }
}
bool decision_cont_boot()
{
  if ()
}
void setup()
{
  Serial.begin(115200);

  bool _temp_state = false;
  bool _last_state = false;

  uint8_t last_boots_result = 255;
  bool FSok = startFS();
  if (FSok)
  {
    setValue(keys[2], SETUP_ERR);
    last_boots_result = last_boot_state(0);
    // if (startFS())
    // {
    //   Serial.println("FS loaded OK");
    //   FSok = true;
    // }
    // else
    // {
    //   Serial.println("FS loaded Failed");
    //   FSok = false;
    // }

    // if (FSok)
    // {
    //   setValue(keys[2], SETUP_ERR);

    //   if (read_prev_boot(0) == BOOT_OK)
    //   {
    //     _last_state = true;
    //     Serial.println("LastBoot - OK");
    //   }
    //   else
    //   {
    //     _last_state = false;
    //     Serial.println("LastBoot - notOK");
    //   }
    // }
    // else
    // {
    //   cont_flag = false;
    // }
  }

  void loop()
  {
    // put your main code here, to run repeatedly:
    loop_bootUpdate(5);
  }
