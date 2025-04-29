/**
* @file
* @brief LED bar functionality
*
*/

#include <msp430fr2310.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t recieved_data = 0;
uint8_t trapdoor_turning_count = 0;
uint8_t current_state = 0;

uint8_t OFF = 0, OPENING = 1, CLOSING = 2;

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // I2C Setup
    UCB0CTLW0 &=~UCSWRST;                                 //clear reset register

    // 1. Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;        // UCSWRST = 1 for eUSCI_B0 in SW reset

    // 2. Configure eUSCI_B0
    UCB0CTLW0 |= UCMODE_3;                //I2C slave mode, SMCLK
    UCB0I2COA0 = 0x0A | UCOAEN;           //SLAVE0 own address is 0x0A| enable

    // 3. Configure Ports as I2C
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2;

    // 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;

    // 5. Enable Interrupts
    UCB0IE |= UCRXIE0;          // Enable I2C Rx0 IRQ

     // Timer B1
    // Math: 1s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 8
    TB1CTL |= TBCLR;        // Clear timer and dividers
    TB1CTL |= TBSSEL__SMCLK;  // Source = SMCLK
    TB1CTL |= MC__UP;       // Mode UP
    TB1CTL |= ID__8;         // divide by 8 
    TB1EX0 |= TBIDEX__5;    // divide by 5 (100)

    TB1CCR0 = 25000;

    TB1CCTL0 &= ~CCIFG;     // Clear CCR0
    TB1CCTL0 |= CCIE;       // Enable IRQ

    // PWM setup for driving motor
    P1DIR |= BIT6;                     // P1.6 and P1.7 output
    P1SEL1 |= BIT6;                    // P1.6 and P1.7 options select

    TB0CCR0 = 21300-1;                         // PWM Period (21.3 ms)
    TB0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB0CCR1 = 1300;                            // CCR1 PWM duty cycle (1.3 ms)
    TB0CTL = TBSSEL__SMCLK | TBCLR;           // SMCLK, up mode, clear TBR
    TB0CTL &= ~MC__UP;                        // setting MC=0 to stop timer

    // LED Setup
    P2DIR |= BIT0;          // Config as Output
    P2OUT |= BIT0;          // turn on to start

    // Buzzer control
    P1DIR |= BIT4;
    P1OUT &= ~BIT4;

    __enable_interrupt();       // Enable Maskable IRQs
    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    while (true)
    {
        
    }
}

//-- Interrupt Service Routines -----------------------

/**
* receiveData
*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void receive_data(void)
{
    switch(UCB0IV)             // determines which IFG has been triggered
    {
    case USCI_I2C_UCRXIFG0:                 // ID 0x16: Rx IFG
        current_state = OPENING;
        recieved_data = UCB0RXBUF;    // retrieve data
        break;
    default:
        break;
    }

}
//----- end receiveData------------

/**
* Heartbeat LED and trapdoor opening/closing
*/
#pragma vector = TIMER1_B0_VECTOR
__interrupt void heartbeat_LED(void)
{
    P2OUT ^= BIT0;          // P2.0 xOR
    switch(current_state)
    {
        case OPENING:
            if(trapdoor_turning_count == 0)
            {
                // Math: .2s = (1*10^-6)(D1)(D2)(5k)    D1 = 5, D2 = 8
                TB1CCR0 = 5000;
                P1OUT |= BIT4;             // turn on buzzer
                TB0CTL |= MC__UP;          // setting MC=1 to start trapdoor
            }
            trapdoor_turning_count++;
            if(trapdoor_turning_count == 6)
            {
                current_state = CLOSING;
                trapdoor_turning_count = 0;
            }
            break;
        case CLOSING:
            if(trapdoor_turning_count == 0)
            {   
                P1OUT &= ~BIT4;             // turn off buzzer
                TB0CTL &= ~MC__UP;          // setting MC=0 to stop trapdoor
                TB0CCR0 = 21700-1;          // PWM Period (21.6 ms)
                TB0CCR1 = 1700;             // CCR1 PWM duty cycle (1.6 ms)
                TB0CTL |= MC__UP;           // setting MC=1 to start trapdoor again
            }
            trapdoor_turning_count++;
            if(trapdoor_turning_count == 6)
            {
                current_state = OFF;
                TB0CTL &= ~MC__UP;          // setting MC=0 to stop trapdoor
                trapdoor_turning_count = 0;
                TB1CCR0 = 25000;
            }
            break;
        default:
            break;
    }
    TB0CCTL0 &= ~CCIFG;     // clear flag
}
// ----- end heartbeatLED-----
