/*
 * part2hardwareDefined.c
 *
 *  Created on: Apr 1, 2023
 *      Author: Lauren Eckert
 *
 *
 * Task:
 * You will need to use pins 6.0, 6.1, and 6.2 to drive an RGB LED.
 * These will need to be configured with a PWM Period of 1ms.
 * You need your RGB LED to cycle between the following colors in order:
 *       Red
 *       Orange (Red + Green)
 *       Green
 *       Cyan (Green + Blue)
 *       Blue
 *       Purple (Red + Blue)
 *
 *  You need to cover colors in between them, meaning as you transition from Red to Orange, it shouldn't be just 2 colors.
 *  The amount of colors are up to you, but is needs to appear smooth in transition.
 *  The timing for cycling is up to you to determine as well.
 *
 *  Amounts for each color:
 *
 *  Red                   R = 50% G = 0%  B = 0%
 *  Orange (Red + Green)  R = 50% G = 50% B = 0%
 *  Green                 R = 0%  G = 50% B = 0%
 *  Cyan (Green + Blue)   R = 0%  G = 50% B = 50%
 *  Blue                  R = 0%  G = 0%  B = 50%
 *  Purple (Red + Blue)   R = 50% G = 0%  B = 50%
 *
 *
 *
 *
 */


#define PERIOD 1000
#define RED_TO_GREEN 0
#define GREEN_TO_BLUE 1
#define BLUE_TO_RED 2

#include <msp430.h>

// State variable for tracking LED transitions
char LEDstate = RED_TO_GREEN;

void LEDSetup();
void TimerSetup();

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    LEDSetup();     // Initialize LED pins
    TimerSetup();   // Initialize timer

    PM5CTL0 &= ~LOCKLPM5;       // Unlock LPM5

    __bis_SR_register(LPM0_bits | GIE); // Enter LPM0 with interrupts enabled
  //  __no_operation();           // For debugger
}

// Configure LED pins and interrupts
void LEDSetup(){
    P6DIR |= BIT0 | BIT1 | BIT2; // Set P6.0-6.2 as output for RGB LED

    P6OUT &= ~(BIT0 | BIT1 | BIT2); // Initialize pins to power-off state

    P6SEL0 |= BIT0 | BIT1 | BIT2; // Select PWM for P6.0-6.2
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2);    //Clearing bits

    P6IE |= BIT0 | BIT1 | BIT2;  // Enable P6.0-6.2 interrupts
}

// Configure timer for PWM and interrupts
void TimerSetup(){
    TB3CCR0 = PERIOD - 1;          // Set PWM period
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR; // Configure timer: SMCLK, up mode, clear TBR

    TB3CCTL1 = OUTMOD_7;           // (RED) Set output mode for CCR1-reset/set
    TB3CCTL2 = OUTMOD_7;           // (GREEN) Set output mode for CCR2-reset/set
    TB3CCTL3 = OUTMOD_7;           // (BLUE) Set output mode for CCR3-reset/set

    TB3CCR1 = PERIOD - 1;          // Set initial duty cycle for RGB LED
    // Set the initial duty cycle values for the green and blue LEDs to 0%
    TB3CCR2 = 0;    // Green LED off
    TB3CCR3 = 0;    // Blue LED off

    TB1CCTL0 |= CCIE;              // Enable TB1 CCR0 overflow interrupt
    TB1CCR0 = 1;                   // Set initial period for TB1
    TB1CTL = TBSSEL_1 | MC_2 | ID_3 | TBCLR | TBIE; // Configure timer: ACLK, continuous mode, divider /8, clear TBR, enable interrupt
}

// Timer B1 ISR for handling LED transitions
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    switch(LEDstate){
    case RED_TO_GREEN:
        TB3CCR1--;                // Decrease RED to 0%
        TB3CCR2++;                // Increase GREEN to 50%
        if (TB3CCR1 == 0) LEDstate = GREEN_TO_BLUE;
        break;
    case GREEN_TO_BLUE:
        TB3CCR2--;                // Decrease GREEN to 0%
        TB3CCR3++;                // Increase BLUE to 50%
        if (TB3CCR2 == 0) LEDstate = BLUE_TO_RED;
        break;
    case BLUE_TO_RED:
        TB3CCR3--;                // Decrease BLUE to 0%
        TB3CCR1++;                // Increase RED to 50%
        if (TB3CCR3 == 0) LEDstate = RED_TO_GREEN;
        break;
    }

    // Check if the timer value TB3R is greater than or equal to 60000
    if(TB3R >= 60000)
        TB3R = 1;                           // If so, reset TB3R to 1 to avoid overflow (max value is 65535)

     // Increment TB1CCR0 by 20 to adjust the timer compare value
    // This determines the speed of the LED color fade transition
    TB1CCR0 += 20;
}
