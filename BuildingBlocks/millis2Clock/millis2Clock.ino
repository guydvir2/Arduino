

// ~~~~~~~~~ generate Uptime ~~~~~~~~
#define SECS_PER_MIN (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY (SECS_PER_HOUR * 24L)

#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) ((_time_ % SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) (_time_ / SECS_PER_DAY)

void calc_time(long val, char *ret_clk)
{
  int days = elapsedDays(val);
  int hours = numberOfHours(val);
  int minutes = numberOfMinutes(val);
  int seconds = numberOfSeconds(val);

  sprintf(ret_clk, "%01dd %02d:%02d:%02d", days, hours, minutes, seconds);
}


void setup()
{
  Serial.begin(9600);
}
void loop()
{
  delay(50);
}
