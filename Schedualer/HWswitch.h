void startOUTPUTSio()
{
  analogWriteRange(PWM_res); /* PWM at ESP8266 */

  for (uint8_t x = 0; x < numSW; x++)
  {
    pinMode(outputPin[x], OUTPUT);
    if (OnatBoot[x])
    {
      TURN_ON(x);
    }
    else
    {
      TURN_OFF(x);
    }

    if (useIndicLED[x])
    {
      pinMode(indicPin[x], OUTPUT);
    }
  }
}
bool getHWstate(uint8_t &i)
{
  if (!outputPWM[i])
  {
    return IS_OUTPUT_ON(i);
  }
  else
  {
    return lastPWMvalue[i] > 0;
  }
}
void PWMdim(int dim_step, uint8_t i)
{
  int _PWMstep = 1;
  int desiredval = CONV_TO_PWM_VALUE(i, dim_step);

  desiredval < lastPWMvalue[i] ? _PWMstep = -1 *_PWMstep : _PWMstep = _PWMstep;

  while (abs(desiredval - lastPWMvalue[i]) >= abs(_PWMstep))
  {
    lastPWMvalue[i] += _PWMstep;
    SET_PWM_VALUE(i, lastPWMvalue[i]);
    delay(1);
  }
}
void HWswitch(uint8_t &i, bool state, uint8_t _pwmVal = 0)
{
  if (!outputPWM[i]) /* Relays or any On-Off switch */
  {
    state ? TURN_ON(i) : TURN_OFF(i);
  }
  else /* PWM */
  {
    _pwmVal == 0 ? PWMdim(i, defPWM[i]) : PWMdim(i, _pwmVal);
  }
}

