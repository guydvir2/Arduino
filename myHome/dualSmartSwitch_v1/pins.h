void set_Pins_to_MCU(uint8_t a, uint8_t inputPins[], uint8_t outputPins[], uint8_t indicPins[])
{
    if (a == 0) /* Error */
    {
        inputPins[0] = 255;
        outputPins[0] = 255;
        indicPins[0] = 255;
    }
    else if (a == 1) /* SONOFF_S26 */
    {
        inputPins[0] = 0;
        outputPins[0] = 12;
        indicPins[0] = 13;
    }
    else if (a == 2) /* SONOFF_mini */
    {
        inputPins[0] = 255;
        outputPins[0] = 12;
        indicPins[0] = 13;
    }
    else if (a == 3) /* MCU */
    {
        inputPins[0] = 14;
        inputPins[1] = 12;
        outputPins[0] = 5;
        outputPins[1] = 4;
        indicPins[0] = 13;
    }
    else if (a == 4) /* ESP-01 */
    {
        inputPins[0] = 2;
        outputPins[0] = 1;
        indicPins[0] = 255;
    }
    else if (a == 5) /* WEMOS */
    {
        inputPins[0] = 5;
        outputPins[0] = 4;
        indicPins[0] = 255;
    }
    else if (a == 6) /* MCU */
    {
        inputPins[0] = 14;
        outputPins[0] = 4;
    }
}
