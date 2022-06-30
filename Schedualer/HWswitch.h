
bool getHWstate(uint8_t &i)
{
  return lightVector[i]->isON();
}
void HWswitch(uint8_t &i, bool state, int _pwmVal = 0)
{
  if (!outputPWM[i]) /* Relays or any On-Off switch */
  {
    state ? lightVector[i]->turnON() : lightVector[i]->turnOFF();
  }
  else /* PWM */
  {
    // _pwmVal == 0 ? PWMdim(i, defPWM[i]) : PWMdim(i, _pwmVal);
  }
}
