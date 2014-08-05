/**
 * For ATTiny85 @8Mhz (remember to "burn bootloader" first)
 *
 * Timer1 used for PWM enforcing, this means that millis() will not work (not that it matters to us, but if you loan code remember this)
 * Timer0 used for duty-cycle enforcing
 *
 * Remember: Boards with 57727H2 manufacturer marking have silkscreen error where pins 1 & 2 are switched  
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

#define MAX_PULSE_LEN 200 // in uSec, note we have only 8 bit timer *and* the pulses may *not* be longer than 200uSec anyway
#define MAX_100HZ_ON_TIME 500 // in uSec, this, basically duty-cycle percentage times 100, do not exceed 5%

volatile unsigned int pulse_on_time;
volatile uint8_t timing_error;
volatile boolean check_duty_cycle;

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

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
        pulse_on_time += TCNT1;
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
    wdt_disable();

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
    // Track duty-cycle
    initTimerCounter0();

    wdt_enable(WDTO_15MS);
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
    // Port is still high, force low and raise problem flag
    if (OUTPUT_PORT & _BV(OUTPUT_BIT))
    {
        OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
        timing_error |= _BV(1);
    }
}

void initTimerCounter0(void)
{
    // Setup for 0.01s delay
    cli();
    TCCR0A = 0;                  //stop the timer
    TCNT0 = 0;                  //zero the timer
    GTCCR = _BV(PSR0);          //reset the prescaler
    OCR0A = 78;      //set the compare value
    TIMSK = _BV(OCIE0A);         //interrupt on Compare Match A
    //start timer, ctc mode, prescaler clk/1023
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS02) | _BV(CS00);
    sei();
}

ISR(TIMER0_COMPA_vect)
{
    check_duty_cycle = true;
    if (TCNT1)
    {
        pulse_on_time += TCNT1;
    }
}



void loop()
{
    if (timing_error)
    {
        // Detach the input interrupt
        detachPcInterrupt(INPUT_PIN);
        // Force output low
        OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
        // Turn LED on
        digitalWrite(LED_PIN, HIGH);
        // Hang and let watchdog reset us in 2s
        wdt_disable();
        wdt_enable(WDTO_2S);
        while (true);
    }
    if (check_duty_cycle)
    {
        check_duty_cycle = false;
        if (pulse_on_time > MAX_100HZ_ON_TIME)
        {
            OUTPUT_PORT &= 0xff ^ _BV(OUTPUT_BIT);
            timing_error |= _BV(2);
        }
        pulse_on_time = 0;
    }
    wdt_reset();
}

