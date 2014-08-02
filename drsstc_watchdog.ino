/**
 * For ATTiny85 @8Mhz
 *
 * Timer1 used for PWM enforcing, millis() does not work.
 *
 * Remember: Boards with 57727H2 marking have silkscreen error where pins 1 & 2 are switched  
 */
#include <PinChangeInterruptSimple.h>

#define INPUT_PIN 1
#define OUTPUT_PIN 2
#define LED_PIN 3
// On ATTiny85 the pin numbers correspond to bits on the only port
#define OUTPUT_PORT PORTB
#define OUTPUT_BIT 2
#define INPUT_PORT PINB
#define INPUT_BIT 1

#define MAX_PULSE_LEN 100 // in uSec, note we have only 8 bit timer *and* the pulses may *not* be longer than 200uSec anyway
// TODO: limit the duty-cycle somehow

volatile uint8_t timing_error;

#include <avr/io.h>
#include <avr/interrupt.h>

void input_check()
{
    if (INPUT_PORT & _BV(INPUT_BIT))
    {
        OUTPUT_PORT |= _BV(OUTPUT_BIT);
        //startimer, ctc mode, prescaler clk/8    
        TCNT1 = 0;                  //zero the timer
        TCCR1 = _BV(CTC1) | _BV(CS12); // Start timer prescaler/8
    }
    else
    {
        OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
        TCCR1 = 0;                  //stop the timer
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
    pinMode(LED_PIN, OUTPUT);
    /**
     * For some reason both of these get called on both edges, go figure...
    attachPcInterrupt(INPUT_PIN, input_high, RISING);
    attachPcInterrupt(INPUT_PIN, input_low, FALLING);
    */
    attachPcInterrupt(INPUT_PIN, input_check, RISING);
    attachPcInterrupt(INPUT_PIN, input_check, FALLING);
    
    // Setup the timer that forces output low if it's too long
    initTimerCounter1();
    // TODO: Setup watchdog (if we can go to usec resolution with it...)
}


// Timer example from http://forum.arduino.cc/index.php?topic=163393.0 (note we do not have millis() anymore since timer1 is used for that on attiny)
void initTimerCounter1(void)
{
    cli();
    TCCR1 = 0;                  //stop the timer
    TCNT1 = 0;                  //zero the timer
    GTCCR = _BV(PSR1);          //reset the prescaler
    OCR1A = MAX_PULSE_LEN;      //set the compare value
    TIMSK = _BV(OCIE1A);         //interrupt on Compare Match A
    //start timer, ctc mode, prescaler clk/8    
    TCCR1 = _BV(CTC1) | _BV(CS12);
    sei();
}

ISR(TIMER1_COMPA_vect)
{
    // Port is still high, force low
    if (OUTPUT_PORT & _BV(OUTPUT_BIT))
    {
        OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
        timing_error |= _BV(1);
    }
}

/*
// For adjusting timer
volatile int matchDelay = 0;
ISR(TIMER1_COMPB_vect)
{
  if(matchDelay <=4) {TCNT1 = 0; matchDelay++;}
  else matchDelay = 0;
}
*/

void loop()
{
    if (timing_error)
    {
        // TODO: Hang and let watchdog reset us ??
        digitalWrite(LED_PIN, HIGH);
    }
    // TODO: keep clearing the watchdog
}

