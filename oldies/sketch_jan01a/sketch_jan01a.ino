#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <NtpClientLib.h>

bool useSerial = true;

//wifi creadentials
const char* ssid;
const char* ssid_0 = "HomeNetwork_2.4G";
const char* ssid_1 = "Xiaomi_D6C8";
const char* password = "guyd5161";
int networkID = 1;  // 0: HomeNetwork,  1:Xiaomi_D6C8
WiFiClient espClient;
//###################################



time_t bootTime;
time_t t = now();
char timeStamp [50];
char dateStamp [50];

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backligh
  lcd.setCursor(0, 0);

  startNetwork();
  time_t bootTime = now();

}

void selectNetwork() {
  if (networkID == 1 ) {
    ssid = ssid_1;
    //                mqtt_server = mqtt_server_1;
  }
  else {
    ssid = ssid_0;
    //                mqtt_server = mqtt_server_0;
  }
}

void startNetwork() {
  long startWifiConnection = millis();

  selectNetwork();
  if (useSerial) {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  // in case of reboot - timeOUT to wifi
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    if (useSerial) {
      Serial.print(".");
    }
  }

  // if wifi is OK

  if (useSerial) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    startNTP();
  }
}

void startNTP() {
  NTP.begin("pool.ntp.org", 2, true);
  NTP.setInterval(1000 * 3600 * 2);
}

void clockString() {
  sprintf(dateStamp, "%02d-%02d-%02d", year(t), month(t), day(t));
  sprintf(timeStamp, "%02d:%02d:%02d", hour(t), minute(t), second(t));

}

void printCenter(char *string, int i = 0) {
  lcd.setCursor((int)(16 - strlen(string)) / 2, i);
  lcd.print(string);

}

void printLeft(char *string, int i = 0) {
  lcd.setCursor(0, i);
  lcd.print(string);

}

void printRight(char *string, int i = 0) {
  lcd.setCursor(16 - strlen(string), i);
  lcd.print(string);

}


void loop() {

  t = now();
  clockString();
  printLeft(dateStamp, 0);
  printRight(timeStamp, 1);

  delay(100);

}
