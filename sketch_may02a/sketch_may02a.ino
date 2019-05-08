#include <ESP8266WiFi.h>
#include <SPI.h>  
#include <TelegramBot.h>  
 
// Initialize Wifi connection to the router  
char ssid[] = "Xiaomi_D6C8";             // your network SSID (name)  
char pass[] = "guyd5161";            // your network key 
// Initialize Telegram BOT  
const char* BotToken = "812406965:AAEaV-ONCIru8ePuisuMfm0ECygsm5adZHs";    // your Bot Teken  
WiFiClient client;  
TelegramBot bot(BotToken,client);  
const int ledPin = 6;  // the number of the LED pin  
void setup() 
{  
 Serial.begin(115200);  
 while (!Serial) {}  //Start running when the serial is open 
 delay(3000);  
 // attempt to connect to Wifi network:  
 Serial.print("Connecting Wifi: ");  
 Serial.println(ssid);  
 WiFi.mode(WIFI_STA);
 while (WiFi.begin(ssid, pass) != WL_CONNECTED) 
       {  
   Serial.print(".");  
   delay(500);  
 }  
 Serial.println("");  
 Serial.println("WiFi connected");  
 bot.begin();  
 pinMode(ledPin, OUTPUT);  
}  
void loop() 
{  
 message m = bot.getUpdates(); // Read new messages  
 if (m.text.equals("On")) 
       {  
   digitalWrite(ledPin, HIGH);  
   Serial.println("message received");  
   bot.sendMessage(m.chat_id, "The Led is now ON");  
 }  
 else if (m.text.equals("Off")) 
       {  
   digitalWrite(ledPin, LOW);  
   Serial.println("message received");  
   bot.sendMessage(m.chat_id, "The Led is now OFF");  
 }  
}  
