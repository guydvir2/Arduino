#include "Arduino.h"
#include "myTelegServer.h"

myTelegram::myTelegram(char *botTok, uint8_t checkServer_interval) : bot(botTok, client)
{
	_botTok = botTok;
	_Bot_mtbs = checkServer_interval;
}
void myTelegram::handleNewMessages(int numNewMessages)
{
	String sendmsg="";

	for (int i = 0; i < numNewMessages; i++)
	{
		String chat_id = String(bot.messages[i].chat_id);
		String text = bot.messages[i].text;
		String from_name = bot.messages[i].from_name;
		if (from_name == "")
			from_name = "Guest";
		_ext_func(text, from_name, chat_id, sendmsg);

		if (sendmsg != "")
		{
			bot.sendMessage(chat_id, sendmsg, "");
		}
	}
}
void myTelegram::begin()
{
	client.setInsecure();
}
void myTelegram::begin(cb_func2 funct)
{
	_ext_func = funct; // call to external function outside of clss
	client.setInsecure();
}
void myTelegram::send_msg(char *msg)
{
	bot.sendMessage(chatID, msg, "");
}
void myTelegram::send_msg(String &msg)
{
	bot.sendMessage(chatID, msg, "");
}
void myTelegram::looper()
{
	if (millis() > _Bot_lasttime + _Bot_mtbs * 1000)
	{
		int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

		while (numNewMessages)
		{
			handleNewMessages(numNewMessages);
			numNewMessages = bot.getUpdates(bot.last_message_received + 1);
		}
		_Bot_lasttime = millis();
	}
}
