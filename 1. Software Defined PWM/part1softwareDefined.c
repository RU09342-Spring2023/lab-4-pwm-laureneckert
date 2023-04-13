/*
 * part1softwareDefined.c
 *
 *  Created on: Apr 1, 2023
 *      Author: Lauren Eckert
 *
 * Task:
 *
 *  - You need to generate a 1kHz PWM signal with a duty cycle between 0% and 100%.
 *  - Upon the processor starting up, you should PWM both of the on-board LEDs at a 50% duty cycle.
 *  - Upon pressing the on-board buttons, the duty cycle of the LEDs should increase by 10%, based on which button you press.
 *  - Once you have reached 100%, your duty cycle should go back to 0% on the next button press.
 *
 *  - Button 2.1 Should control LED 1.0
 *  - Button 4.3 should control LED 6.6
 *
 */

#include <msp430.h>

void LEDSetup();
void ButtonSetup();
void TimerB0Setup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;       // Disable high-impedance mode

    LEDSetup();     // Initialize LEDs
    ButtonSetup();  // Initialize buttons
    TimerB0Setup(); // Initialize Timer0
    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 with interrupts
}

// Set up LEDs
void LEDSetup()
{
    // Configure red LED (P1.0) as output
    P1OUT &= ~BIT0; // Clear P1.0 output latch
    P1DIR |= BIT0;  // Set P1.0 as output

    // Configure green LED (P6.6) as output
    P6OUT &= ~BIT6; // Clear P6.6 output latch
    P6DIR |= BIT6;  // Set P6.6 as output
}

// Set up buttons
void ButtonSetup()
{
    // Configure button P2.3 as input with pull-up resistor
    P2OUT |= BIT3;  // Pull-up P2.3
    P2REN |= BIT3;  // Enable P2.3 pull-up resistor
    P2IES &= ~BIT3; // Set P2.3 low-to-high edge
    P2IE |= BIT3;   // Enable P2.3 interrupt

    // Configure button P4.1 as input with pull-up resistor
    P4OUT |= BIT1;  // Pull-up P4.1
    P4REN |= BIT1;  // Enable P4.1 pull-up resistor
    P4IES &= ~BIT1; // Set P4.1 low-to-high edge
    P4IE |= BIT1;   // Enable P4.1 interrupt
}

// Set up Timer B0
void TimerB0Setup()
{
    // Configure Timer B0
    TB0CTL = TBSSEL_2 | MC__UP | TBCLR | TBIE; // SMCLK, up mode, clear
    TB0CCTL0 = OUTMOD_7; // CCR1 reset/set
    TB0CCTL1 |= CCIE; // Enable TB0 CCR2 interrupt
    TB0CCTL2 |= CCIE; // Enable TB0 CCR3 interrupt
    TB0CCR0 = 1000; // Set 1kHz frequency
    TB0CCR1 = 500;  // Set 50% duty cycle for red LED
    TB0CCR2 = 500;  // Set 50% duty cycle for green LED
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3; // Clear P2.3 IFG

    if(TB0CCR1 >= 1000)
        TB0CCR1 = 0;          // Reset duty cycle
    else
        TB0CCR1 += 100;        // Increment duty cycle by 10%
}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1; // Clear P4.1 IFG

    if(TB0CCR2 >= 1000)
        TB0CCR2 = 0;          // Reset duty cycle
    else
        TB0CCR2 += 100;        // Increment duty cycle by 10%
}

// Timer0 B0 interrupt service routine
#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer0_B1_ISR(void)
{
    switch(__even_in_range(TB0IV, TB0IV_TBIFG))
    {
        case  0:                  // No interrupt pending
            break;
        case  TB0IV_TBCCR1:       // CCR1
            P1OUT &= ~BIT0;       // Set P1.0 to 0
            break;
        case  TB0IV_TBCCR2:       // CCR2
            P6OUT &= ~BIT6;       // Set P6.6 to 0
            break;
        case  TB0IV_TBIFG:        // TBIFG
            P1OUT ^= BIT0;        // Toggle red LED
            P6OUT ^= BIT6;        // Toggle green LED
            break;
        default:
            break;
    }
}




