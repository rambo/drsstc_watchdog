#include <PinChangeInterruptSimple.h>

#define INPUT_PIN 2
#define OUTPUT_PIN 1

#define MAX_PULSE_LEN 200 // in uSec
// TODO: limit the duty-cycle somehow

void input_high()
{
}

void input_low()
{
}


void setup()
{
    // put your setup code here, to run once:
    attachPcInterrupt(2, input_high, RISING);
    attachPcInterrupt(2, input_low, FALLING);
}

void loop()
{
    // put your main code here, to run repeatedly: 
  
}

