#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
char incoming_radio_txt[32];// = "";

void startRadio_listening() {
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MIN; RF24_PA_MAX
  radio.setChannel(35);
  radio.setAutoAck(0);

  radio.openReadingPipe(0, address);
  radio.startListening();
}

//~~~~~~~~~~~~~ LCD Display ~~~~~~~~~~~~
#include <LiquidCrystal_I2C.h>
int lcdColumns = 16;
int lcdRows = 2;
char buf[21];


LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void start_LCD () {
  lcd.init();
  lcd.backlight();
}

void LCD_loop() {
  lcd.setCursor(0, 0);
  lcd.print(incoming_radio_txt);
  lcd.setCursor(0, 1);
  lcd.print(buf);
}

void Radio_loop() {
  if (radio.available()) {
    radio.read(&incoming_radio_txt, sizeof(incoming_radio_txt));
    Serial.println(incoming_radio_txt);
  }
}

void ON_time() {
  unsigned long runMillis = millis();
  unsigned long allSeconds = millis() / 1000;
  int runHours = allSeconds / 3600;
  int secsRemaining = allSeconds % 3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining % 60;

  sprintf(buf, "local: %02d:%02d:%02d", runHours, runMinutes, runSeconds);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Reciever Begin");
  startRadio_listening();
  Serial.print("Chip_OK: ");
  Serial.println(radio.isChipConnected());
  start_LCD();
}
void loop() {
  Radio_loop();
  ON_time();
  LCD_loop();
  delay(100);
}
