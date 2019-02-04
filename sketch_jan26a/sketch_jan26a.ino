/*
 *  ESP8266 Communication and Protocols
 *  SPIFFS Direct File Upload Example
 *  -Manoj R. Thkuar
 */
 
#include <ESP8266WiFi.h>
#include <FS.h>   //Include File System Headers
 
const char* file = "/notes.txt";   //Enter your file name
 
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
 
  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");
 
  File dataFile = SPIFFS.open(file, "r");   //Open File for reading
  Serial.println("Reading Data from File:");
  //Data from file
  for(int i=0;i<dataFile.size();i++) //Read upto complete file size
  {
    Serial.print((char)dataFile.read());    //Read file
  }
  dataFile.close();
}
 
void loop() {
}
