// #include <FS.h>
// #include <LittleFS.h>
// #include <ESP8266WiFi.h>
#include <myIOT2.h>
#include <myLOG.h>
flashLOG clkLOG("/abc.txt");

unsigned long T = 1234567890;
myIOT2 iot;

// char *_logfilename = "/logfile.txt";
// int _buff_i = 0;
// #define TEMP_LOG_SIZE 10
// #define TEMP_LOG_LEN 150
// int _logSize = TEMP_LOG_SIZE;                 // entries
// int _logLength = TEMP_LOG_LEN;                // chars in each entry
// char _logBuffer[TEMP_LOG_SIZE][TEMP_LOG_LEN]; // Temp buffer for delayed write
// const char _EOL = '\n';
// unsigned long lastUpdate = 0;

// void insert_log_entry()
// {
//   char a[25];
//   sprintf(a, "%d", T);
//   write(a, true);
// }
// void rawPrintfile()
// {
//   int row_counter = 0;

//   File file = SPIFFS.open(_logfilename, "r");
//   if (!file)
//   {
//     Serial.println("Failed to open file for reading");
//   }
//   Serial.print("~~~ Saved in ");
//   Serial.print(_logfilename);
//   Serial.println(" ~~~");

//   while (file.available())
//   {
//     String line = file.readStringUntil(_EOL);
//     String lineFormat = "row #" + String(row_counter) + " {" + line + "}";
//     Serial.println(lineFormat);
//     row_counter++;
//   }
//   Serial.println("~~~ EOF ~~~");
//   file.close();
// }
// int getnumlines()
// {
//   int row_counter = 0;

//   File file = SPIFFS.open(_logfilename, "r");
//   if (file)
//   {
//     while (file.available())
//     {
//       file.readStringUntil(_EOL);
//       row_counter++;
//     }
//   }
//   file.close();
//   return row_counter;
// }
// void write(const char *message, bool NOW)
// {
//   sprintf(_logBuffer[_buff_i], "%s", message);

//   Serial.print("add buffer #");
//   Serial.print(_buff_i);
//   Serial.print(": ");
//   Serial.println(_logBuffer[_buff_i]);

//   _buff_i++;
//   lastUpdate = millis();

//   if (NOW == true)
//   {
//     _write2file();
//   }
// }
// void _write2file()
// {
//   int num_lines = getnumlines();

//   Serial.print("total Lines before: ");
//   Serial.println(num_lines);
//   Serial.print("entries in buffer: ");
//   Serial.println(_buff_i);

//   File file1 = SPIFFS.open(_logfilename, "a+");
//   if (!file1)
//   {
//     Serial.println("Failed to open file for appending");
//   }
//   else
//   {
//     for (int x = 0; x < _buff_i; x++)
//     {
//       file1.println(_logBuffer[x]);
//     }
//     _buff_i = 0;
//     lastUpdate = 0;
//   }
//   file1.close();
// }

void setup()
{

  Serial.begin(115200);
  Serial.println("\n\nStart!");
  // SPIFFS.begin();
  // insert_log_entry();
  // rawPrintfile();

  clkLOG.start(10, 100);
  uint8_t x = 0;
  // while (x < 10)
  // {
  clkLOG.write("Boot", true);
  //   x++;
  //   delay(500);
  // }
}

void loop()
{
  delay(50);
  if (millis() > 10000)
  {
    iot.sendReset();
  }
  // static long lastW=0;
  // if(millis() - lastW >10000){
  //   lastW=millis();
  //   clkLOG.write("234234");
  // }
  // if(millis()>30000){
  //   ESP.reset();
  // }

  // clkLOG.looper(5);
}