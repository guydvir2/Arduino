// I'm using an ESP8266 to code using Arduino. I'm saving a parameter file on flash. Parameter file can be change without changing code.

// My question is - do I have to save the value extracted from flash and only then to point it to its final destination inside a class? since that value, the pointer is pointed to lives only inside `readParameters_fromFlash()`

// **Example:**

A a_instance;
char array_value[30];

// returns key's value stored in flash.
// Alternative_1
const char *extract_json(const char *key)
{
    StaticJsonDocument<200> doc;
    File readFile = LittleFS.open(_filename, "r");
    DeserializationError error = deserializeJson(DOC, readFile);
    readFile.close();

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        const char *sensor = "ERR";
        return;
    }
    const char *sensor = doc["key"];
    return sensor;
}
// Alternative_2
void char *extract_json(const char *key, const char *value)
{
    StaticJsonDocument<200> doc;
    File readFile = LittleFS.open(_filename, "r");
    DeserializationError error = deserializeJson(DOC, readFile);
    readFile.close();

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        value = "ERR";
        return;
    }
    value = doc["key"];
    return sensor;
}

void readParameters_fromflash()
{
    /* a_instance.a is a const char* variable */
    a_instance.a = extract_json("key");
}

// or should it be implemented as:
// Alternative_3
void readParameters_fromflash()
{
    /* a_instance.a is a const char* variable */
    strcpy(array_value, extract_json("key"));
    a_instance.a = array_value;
}