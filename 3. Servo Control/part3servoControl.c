/*
 * part3servoControl.c
 *
 *  Created on: Apr 1, 2023
 *      Author: Lauren Eckert
 *
 *  Task:
 *  You can chose whichever pin you want to control your Servo (but I might recommend one with hardware PWM control).
 *  Your will use the two buttons on your development board to control the position of your servo.
 *  The button on the left side of the board should turn the servo counterclockwise,
 *  the button on the right side of the board should turn it clockwise.
 *
 *  The servo will have a limit to the amount of degree it can rotate, so make sure you take a look at that before coding.
 *
 *  The servo will need to be most likely powered from the power supply on the bench.
 *  If you do this, you need to make sure you connect common ground between the supply and your board.
 *  Otherwise, your system will not work or you risk damaging your board.
 *
 *
 *      Button 2.1 duty cycle -=100
 *      Button 4.3 duty cycle +=100
 */

#include <msp430.h>

void pinSetup();
void ButtonSetup();
void TimerB3Setup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;  // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings


    pinSetup();                                  // Initialize the pin for servo motor
    ButtonSetup();                               // Initialize buttons
    TimerB3Setup();                              // Initialize Timer3
    __bis_SR_register(LPM0_bits + GIE);          // Enter LPM0 w/ interrupt
}

void pinSetup(){
    P6OUT &= ~BIT0;     // Set P6.0 output to LOW
    P6DIR |= BIT0;      // Set P6.0 as output
    P6SEL0 |= BIT0;     // Configure P6.0 for peripheral function (Timer B3 capture/compare input)
    P6SEL1 &= ~BIT0;    // Configure P6.0 for peripheral function (Timer B3 capture/compare input)
}

void ButtonSetup (){
    // Configure Button on P2.3 as input with pullup resistor
             P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
             P2REN |= BIT3;                          // P2.3 pull-up register enable
             P2IES &= ~BIT3;                         // P2.3 Low --> High edge
             P2IE |= BIT3;                           // P2.3 interrupt enabled
    // Configure Button on P4.1 as input with pullup resistor
             P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
             P4REN |= BIT1;                          // P4.1 pull-up register enable
             P4IES &= ~BIT1;                         // P4.1 Low --> High edge
             P4IE |= BIT1;                           // P4.1 interrupt enabled
}

void TimerB3Setup(){
    // Configure Timer_B3
    TB3CTL = TBSSEL_2 | MC__UP | TBCLR | TBIE;    // SMCLK, up mode, clear TBR, enable interrupt
    TB3CCTL0 = OUTMOD_7;                         // CCR1 reset/set
    TB3CCTL1 |= CCIE;                            // Enable TB0 CCR2 Interrupt
    TB3CCR0 = 20000;                             // 50Hz frequency
    TB3CCR1 = 1500;                              // Initial duty cycle of 1.5 ms
}

// Port 2 ISR
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                         // Clear P2.3 interrupt flag

    if(TB3CCR1 > 1000)
        TB3CCR1 -= 100;                     // Decrease the duty cycle by 5% to move the servo left
    else
        TB3CCR1 = TB3CCR1;                  // If the servo is already at its minimum position, do not move it further

}                                          // End of Port_2 ISR

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 interrupt flag

    if(TB3CCR1 < 2000)
        TB3CCR1 += 100;                     // Increase the duty cycle by 5% to move the servo right
    else
        TB3CCR1 = TB3CCR1;                  // If the servo is already at its maximum position, do not move it further

}                                          // End of Port_4 ISR

// TimerB3 interrupt service routine
#pragma vector = TIMER3_B1_VECTOR
__interrupt void Timer3_B1_ISR(void)
{
    switch(__even_in_range(TB3IV,TB3IV_TBIFG))
    {
        case  TB3IV_TBCCR1:                 // CCR1 interrupt
            P6OUT &= ~BIT0;                 // Set P6.0 to low to end the PWM cycle
            break;
        default:
            break;
    }

}                                          // End of TimerB3 ISR





