#define SW_UP_PIN 2
#define SW_DOWN_PIN 3

#define SW_PRESSED LOW
volatile bool UP_STATE_IRQ = false;
volatile bool DOWN_STATE_IRQ = false;
volatile bool CHANGE_SW_IRQ = false;

void UP_ISR()
{
  CHANGE_SW_IRQ = true;
  UP_STATE_IRQ = digitalRead(SW_UP_PIN);
//  detachInterrupt(digitalPinToInterrupt(SW_DOWN_PIN));
//  detachInterrupt(digitalPinToInterrupt(SW_UP_PIN));
}
void DOWN_ISR()
{
  CHANGE_SW_IRQ = true;
  DOWN_STATE_IRQ = digitalRead(SW_DOWN_PIN);
//  detachInterrupt(digitalPinToInterrupt(SW_DOWN_PIN));
//  detachInterrupt(digitalPinToInterrupt(SW_UP_PIN));
}
void start_IRQ()
{
  UP_STATE_IRQ = digitalRead(SW_UP_PIN);
  DOWN_STATE_IRQ = digitalRead(SW_DOWN_PIN);
  attachInterrupt(digitalPinToInterrupt(SW_DOWN_PIN), DOWN_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW_UP_PIN), UP_ISR, CHANGE);
}

void start_gpio()
{
  pinMode(SW_UP_PIN, INPUT_PULLUP);
  pinMode(SW_DOWN_PIN, INPUT_PULLUP);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  start_gpio();
  start_IRQ();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (CHANGE_SW_IRQ)
  {
    if (DOWN_STATE_IRQ == SW_PRESSED)
    {
      Serial.println("down");
    }
    else if (UP_STATE_IRQ == SW_PRESSED)
    {
      Serial.println("up");
    }
    else
    {
      Serial.println("off");
    }
    CHANGE_SW_IRQ = false;
//    attachInterrupt(digitalPinToInterrupt(SW_DOWN_PIN), DOWN_ISR, CHANGE);
//    attachInterrupt(digitalPinToInterrupt(SW_UP_PIN), UP_ISR, CHANGE);
  }
  delay(50);

}
