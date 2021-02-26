extern const char *dev_name;

const char *gKeys[] = {"from", "type", "key0", "key2", "sent"};
const char *m_types[] = {"q", "ans", "cmd", "info", "act"};
const char *questions[] = {"clk", "wakeTime", "whois_online"};
const char *wakes[] = {"from", "type", "wake#", "Vbat", "sleep"};

void createMSG_JSON(char a[], const char *v0, const char *v1 = nullptr, const char *v2 = nullptr, const char *v3 = nullptr,
                    const char *v4 = nullptr, const char *key[] = gKeys)
{
  /* 3 key/value pairs */
  if (v4 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2, key[3], v3, key[4], v4);
  }
  else if (v3 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2, key[3], v3);
  }
  else if (v2 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2);
  }
  else if (v1 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1);
  }
  else if (strcmp(v1, nullptr) == 0)
  {
    sprintf(a, "{\"%s\":\"%s\"}", key[0], v0);
  }
}
bool gen_send(const char *key0, const char *value0, const char *key1 = nullptr, const char *value1 = nullptr,
              const char *key2 = nullptr, const char *value2 = nullptr, const char *key3 = nullptr, const char *value3 = nullptr,
              const char *key4 = nullptr, const char *value4 = nullptr)
{
  /* 4 pairs of key/values */
  char outmsg[250];
  const char *k[] = {key0, key1, key2, key3, key4};
  createMSG_JSON(outmsg, value0, value1, value2, value3, value4, k);

  /* Sending the message */
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    if (PRINT_MESSAGES)
    {
      Serial.print(">>[OK] ");
      Serial.println(outmsg);
    }
    return 1;
  }
  else
  {
    if (PRINT_MESSAGES)
    {
      Serial.print(">>[F] ");
      Serial.println(outmsg);
    }
    return 0;
  }
}

bool send(const char *msg_type, const char *p0, const char *p1 = nullptr, const char *p2 = nullptr, const char *key[] = gKeys)
{
  char outmsg[250];

  /* Sending the message */
  if (gen_send(key[0], dev_name, key[1], msg_type, key[2], p0, key[3], p1, key[4], p2))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}