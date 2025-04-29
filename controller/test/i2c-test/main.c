// port interrupt test
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "intrinsics.h"
#include "msp430fr2355.h"

uint8_t current_state = 0, reset_counter = 0, ir_flag = 0, button_pressed = 0,
trapdoor_wait_flag = 0, trapdoor_wait_count = 0, code_wait_flag = 0, code_wait_count = 0;
// starting values for every pattern
const uint8_t TRAPDOOR_ADDR = 0x0A, LCD_ADDR = 0x0B, START = 0, 
REAL_ENTER = 1, WELCOME = 2, GOODBYE = 3, BLANK = 4;
char cur_char;

void transmit_to_lcd()
{
    UCB0TBCNT = 1;
    UCB0I2CSA = LCD_ADDR;
    while (UCB0CTLW0 & UCTXSTP);                      // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR | UCTXSTT;                      // I2C TX, start condition
}

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

 // Configure Pins for I2C
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2 | BIT3;                            // I2C pins

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                             // put eUSCI_B in reset state
    UCB0CTLW0 |= UCMODE_3 | UCMST;                    // I2C master mode, SMCLK
    UCB0CTLW1 |= UCASTP_2;                            // Automatic stop after hit TBCNT
    UCB0I2CSA = LCD_ADDR;                         // configure slave address
    UCB0BRW = 0x8;                                    // baudrate = SMCLK / 8
    UCB0TBCNT = 1;                                    // num bytes to recieve

    UCB0CTLW0 &=~ UCSWRST;                            // clear reset register
    UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE0 | UCBCNTIE;// transmit, receive, TBCNT, an


  __enable_interrupt();   // enable maskable IRQs 
  PM5CTL0 &= ~LOCKLPM5;   // turn on GPIO
  __no_operation();                         // For debug

  current_state = START;

  while(1)
  {
    __delay_cycles(100000);
    transmit_to_lcd();
  }
}

// /**
// * transmit_data
// */
// #pragma vector = EUSCI_B0_VECTOR
// __interrupt void transmit_data(void)
// {
//     switch(UCB0IV)             // determines which IFG has been triggered
//     {
//     case USCI_I2C_UCNACKIFG:
//         UCB0CTL1 |= UCTXSTT;                      //resend start if NACK
//         break;                                      // Vector 4: NACKIFG break
//     case USCI_I2C_UCTXIFG0:
//         if(UCB0I2CSA == LCD_ADDR)
//         {
//             UCB0TXBUF = current_state;            // Load TX buffer
//         }
//         else
//         {
//             // register address begins at 0, then we set sec and min to 0
//             UCB0TXBUF = 1;                          
//         }
//         break;                                  
//     default:
//         break;
//     }
// }


/**
* transmit and recieve data
*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void transmit_data(void)
{
    switch(UCB0IV)             // determines which IFG has been triggered
    {
    case USCI_I2C_UCNACKIFG:
        UCB0CTL1 |= UCTXSTT;                      //resend start if NACK
        break;                                      // Vector 4: NACKIFG break
    case USCI_I2C_UCTXIFG0:
        if(UCB0I2CSA == LCD_ADDR)
        {
            UCB0TXBUF = current_state;            // Load TX buffer
        }
        else
        {
            // register address begins at 0, then we set sec and min to 0
            UCB0TXBUF = 1;                          
        }
        break;                                    
    default:
        break;
    }

}
