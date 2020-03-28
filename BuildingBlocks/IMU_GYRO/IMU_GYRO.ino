#include <Wire.h>
#include <MechaQMC5883.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

Adafruit_MPU6050 mpu;
MechaQMC5883 qmc;
LiquidCrystal_I2C lcd(0x27, 20, 4); 

sensors_event_t a, g, temp;
int x, y, z, azimuth;

void setupIMU()
{
  Serial.println("MPU6050 OLED demo");
  if (!mpu.begin())
  {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  Serial.println("Found a MPU-6050 sensor");
}

void IMUloop()
{
  mpu.getEvent(&a, &g, &temp);
  // Serial.println(" ~~~~ IMU ~~~~\n");

  // Serial.print("Accelerometer ");
  // Serial.print("X: ");
  // Serial.print(a.acceleration.x, 1);
  // Serial.print(" m/s^2, ");
  // Serial.print("Y: ");
  // Serial.print(a.acceleration.y, 1);
  // Serial.print(" m/s^2, ");
  // Serial.print("Z: ");
  // Serial.print(a.acceleration.z, 1);
  // Serial.println(" m/s^2");

  // Serial.print("Gyroscope ");
  // Serial.print("X: ");
  // Serial.print(g.gyro.x, 1);
  // Serial.print(" rps, ");
  // Serial.print("Y: ");
  // Serial.print(g.gyro.y, 1);
  // Serial.print(" rps, ");
  // Serial.print("Z: ");
  // Serial.print(g.gyro.z, 1);
  // Serial.println(" rps");
}

void setup_MAGNO()
{
  Wire.begin();
  qmc.init();
  //qmc.setMode(Mode_Continuous,ODR_200Hz,RNG_2G,OSR_256);
}

void setup_LCD()
{
  lcd.init();
  lcd.backlight();
}

void loop_MAGNO()
{
  //float azimuth; //is supporting float too
  qmc.read(&x, &y, &z, &azimuth);
  //azimuth = qmc.azimuth(&y,&x);//you can get custom azimuth
  // Serial.println(" ~~~~ Magnetometer ~~~~\n");
  // Serial.print("x: ");
  // Serial.print(x);
  // Serial.print(" y: ");
  // Serial.print(y);
  // Serial.print(" z: ");
  // Serial.print(z);
  // Serial.print(" a: ");
  // Serial.print(azimuth);
  // Serial.println();

  // sprintf(magno, "Azimuth: %d deg", azimuth);
  // lcd.setCursor(0, 0);
  // lcd.print(magno);
  // delay(100);
}

void LCD_output()
{
  int d1 = 0;
  int d2 = 7;
  int d3 = 12;

  char aa[3][10];
  char gg[3][10];

  dtostrf(a.acceleration.x, 4, 1, aa[0]);
  dtostrf(a.acceleration.y, 4, 1, aa[1]);
  dtostrf(a.acceleration.z, 4, 1, aa[2]);

  dtostrf(g.gyro.x, 4, 1, gg[0]);
  dtostrf(g.gyro.y, 4, 1, gg[1]);
  dtostrf(g.gyro.z, 4, 1, gg[2]);

  lcd.setCursor(d1, 0);
  lcd.print("m/s^2");
  lcd.setCursor(d2, 0);
  lcd.print("rpm");
  lcd.setCursor(d3, 0);
  lcd.print("*C");

  for (int x = 0; x < 3; x++)
  {
    lcd.setCursor(d1, x + 1);
    lcd.print(aa[x]);

    lcd.setCursor(d2, x + 1);
    lcd.print(gg[x]);
  }
  lcd.setCursor(d3, 1);
  lcd.print(temp.temperature);

  lcd.setCursor(d3, 2);
  lcd.print("Azm");
  lcd.setCursor(d3, 3);
  lcd.print(azimuth);
}

void setup()
{
  Serial.begin(9600);
  setupIMU();
  setup_MAGNO();
  setup_LCD();
}

void loop()
{
  IMUloop();
  loop_MAGNO();
  LCD_output();

  delay(100);
}
