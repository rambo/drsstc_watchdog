/**
 * For ATTiny85
 *
 * 
 */
#include <PinChangeInterruptSimple.h>

#define INPUT_PIN 1
#define OUTPUT_PIN 2
// On ATTiny85 the pin numbers correspond to bits on the only port
#define OUTPUT_PORT PORTB
#define OUTPUT_BIT 2
#define INPUT_PORT PINB
#define INPUT_BIT 1


#define MAX_PULSE_LEN 200 // in uSec
// TODO: limit the duty-cycle somehow


void input_check()
{
    if (INPUT_PORT & _BV(INPUT_BIT))
    {
        OUTPUT_PORT |= _BV(OUTPUT_BIT);
    }
    else
    {
        OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
    }
}


void input_high()
{
    OUTPUT_PORT |= _BV(OUTPUT_BIT);
}

void input_low()
{
    OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
}


void setup()
{
    // TODO: disable watchdog
    pinMode(OUTPUT_PIN, OUTPUT);
    /**
     * For some reason both of these get called on both edges, go figure...
    attachPcInterrupt(INPUT_PIN, input_high, RISING);
    attachPcInterrupt(INPUT_PIN, input_low, FALLING);
    */
    attachPcInterrupt(INPUT_PIN, input_check, RISING);
    attachPcInterrupt(INPUT_PIN, input_check, FALLING);
    // TODO: Setup watchdog (if we can go to usec resolution with it...)
}

void loop()
{
    // TODO: keep clearing the watchdog
    // TODO: unless we can enforce max pulse_len with timer keep polling
}

