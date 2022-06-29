void ON_OFF_on_handle(Button2 &b)
{
  uint8_t i = b.getID();
  ONcmd(i, timeouts[i], SRCS[BTN]);
}
void ON_OFF_off_handle(Button2 &b)
{
  OFFcmd(b.getID(), SRCS[BTN]);
}
void Momentary_handle(Button2 &b)
{
  uint8_t i = b.getID();
  if (getHWstate(i) == true) /* Turn OFF if ON */
  {
    OFFcmd(i, SRCS[BTN]);
  }
  else
  {
    ONcmd(i, timeouts[i], SRCS[BTN]);
  }
}
void TrigSensor_handler(Button2 &b)
{
  const uint8_t update_timeout = 30; // must have passed this amount of seconds to updates timeout
  uint8_t i = b.getID();
  unsigned int _remaintime = remainWatch(i);

  if (_remaintime != 0 && timeouts[i] - _remaintime > update_timeout)
  {
    startWatch(i); /* Restart timeout after 30 sec */
  }
}
void startButtons()
{
  for (uint8_t i = 0; i < numSW; i++)
    if (useInputs[i])
    {
      buttonVector[i]->begin(inputPin[i]);
      buttonVector[i]->setID(i);

      if (trigType[i] == MOMENTARY)
      {
        buttonVector[i]->setPressedHandler(Momentary_handle);
      }
      else if (trigType[i] == ON_OFF)
      {
        buttonVector[i]->setPressedHandler(ON_OFF_on_handle);
        buttonVector[i]->setReleasedHandler(ON_OFF_off_handle);
      }
      else if (trigType[i] == TRIGGER_SERNSOR)
      {
        buttonVector[i]->setPressedHandler(TrigSensor_handler);
      }
      else if (trigType[i] == BTN_TO_PWM)
      {
      }
      else if (trigType[i] == MULTI_PRESS)
      {
      }
    }
}
void Button_looper()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    if (useInputs[i])
    {
      buttonVector[i]->loop();
    }
  }
}