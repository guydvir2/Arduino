#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <ArduinoJson.h>

#include <myIOT.h>

// Initialize Wifi connection to the router
char ssid[] = SSID_ID;       // your network SSID (name)
char password[] = PASS_WIFI; // your network key

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN_3, secured_client);

int Bot_mtbs = 1000; // mean time between scan messages
long Bot_lasttime;   // last time messages' scan has been done
#define LOG_SIZE
DynamicJsonDocument logDoc(1500);

int bulk_messages_mtbs = 1500;      // mean time between send messages, 1.5 seconds
int messages_limit_per_second = 25; // Telegram API have limit for bulk messages ~30 messages per second

String eventlog_filename = "eventlog.json";

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/start")
    {
      if (addlog_entry(chat_id, from_name))
      {
        String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
        welcome += "This is Bulk Messages example.\n\n";
        welcome += "/showallusers : show all subscribed users\n";
        welcome += "/testbulkmessage : send test message to subscribed users\n";
        welcome += "/removeallusers : remove all subscribed users\n";
        welcome += "/stop : unsubscribe from bot\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      }
      else
      {
        bot.sendMessage(chat_id, "Something wrong, please try again (later?)", "");
      }
    }

    if (text == "/stop")
    {
      if (removelog_entry(chat_id))
      {
        bot.sendMessage(chat_id, "Thank you, " + from_name + ", we always waiting you back", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Something wrong, please try again (later?)", "");
      }
    }

    if (text == "/testbulkmessage")
    {
      sendMessageToAllSubscribedUsers("ATTENTION, this is bulk message for all subscribed users!");
    }

    if (text == "/showallusers")
    {
      File logfile = SPIFFS.open("/" + eventlog_filename, "r");

      if (!logfile)
      {
        bot.sendMessage(chat_id, "No subscription file", "");
      }

      size_t size = logfile.size();

      if (size > 1024)
      {
        bot.sendMessage(chat_id, "Subscribed users file is too large", "");
      }
      else
      {
        String file_content = logfile.readString();
        bot.sendMessage(chat_id, file_content, "");
      }
    }

    if (text == "/removeallusers")
    {
      if (SPIFFS.remove("/" + eventlog_filename))
      {
        bot.sendMessage(chat_id, "All users removed", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Something wrong, please try again (later?)", "");
      }
    }
  }
}

JsonObject getdata_log()
{
  File logfile = SPIFFS.open("/" + eventlog_filename, "r");

  if (!logfile)
  {
    Serial.println("Failed to open log file");

    // Create empty file (w+ not working as expect)
    File f = SPIFFS.open("/" + eventlog_filename, "w");
    f.close();

    JsonObject log_entries;

    return log_entries;
  }
  else
  {

    size_t size = logfile.size();

    if (size > 1500)
    {
      Serial.println(" log_entries file is too large");
      //return log_entries;
    }

    String file_content = logfile.readString();
    DeserializationError error = deserializeJson(logDoc, file_content);
    JsonObject log_entries = logDoc.to<JsonObject>();
    serializeJson(logDoc, Serial);
    if (error)
    {
      Serial.println("Failed to parse subscribed users file");
      return log_entries;
    }

    logfile.close();
    return log_entries;
  }
}

bool addlog_entry(String log_id, String log_entry)
{
  JsonObject logs = getdata_log();

  File logfile = SPIFFS.open("/" + eventlog_filename, "w+");

  if (!logfile)
  {
    Serial.println("Failed to open subscribed logs file for writing");
    //return false;
  }

  // logs.getOrCreateMember(log_id, log_entry);
  logs[log_id] = log_entry;
  serializeJson(logs, logfile);

  logfile.close();

  return true;
}

bool removelog_entry(String log_id)
{
  JsonObject logs = getdata_log();

  File logfile = SPIFFS.open("/" + eventlog_filename, "w");

  if (!logfile)
  {
    Serial.println("Failed to open subscribed logs file for writing");
    return false;
  }

  logs.remove(log_id);
  serializeJson(logs, logfile);
  logfile.close();

  return true;
}

void sendMessageToAllSubscribedUsers(String message)
{
  int users_processed = 0;

  JsonObject logs = getdata_log();

  for (JsonObject::iterator it = logs.begin(); it != logs.end(); ++it)
  {
    users_processed++;

    if (users_processed < messages_limit_per_second)
    {
      const char *chat_id = it->key().c_str();
      bot.sendMessage(chat_id, message, "");
    }
    else
    {
      delay(bulk_messages_mtbs);
      users_processed = 0;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
  else
  {
    Serial.println("SPIFFS mounted OK");
  }

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // secured_client.setInsecure();
  addlog_entry("A", "GUYDVIR");
  addlog_entry("B", "GUYDVIR2");
  addlog_entry("C", "GUYDVIR3");

  // JsonObject logs = getdata_log();

  // serializeJson(logs, Serial);
}

void loop()
{
  // if (millis() > Bot_lasttime + Bot_mtbs)
  // {
  //   int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  //   while (numNewMessages)
  //   {
  //     Serial.println("got response");
  //     handleNewMessages(numNewMessages);
  //     numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  //   }

  //   Bot_lasttime = millis();
  // }
}
