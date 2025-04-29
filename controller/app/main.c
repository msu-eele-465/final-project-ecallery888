/**
* @file
* @brief Implements LED_status and keypad to operate a pattern-displaying LED bar
*
*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/keypad.h"
#include "intrinsics.h"
#include "msp430fr2355.h"


uint8_t current_state = 0, reset_counter = 0, ir_flag = 0, button_pressed = 0,
trapdoor_wait_flag = 0, trapdoor_wait_count = 0, code_wait_flag = 0, code_wait_count = 0;
// starting values for every pattern
const uint8_t TRAPDOOR_ADDR = 0x0A, LCD_ADDR = 0x0B, START = 0, 
REAL_ENTER = 1, WELCOME = 2, GOODBYE = 3, BLANK = 4;
char cur_char;

// global keypad and pk_attempt initialization
Keypad keypad = {
    .lock_state = LOCKED,                           // locked is 1
    .row_pins = {BIT3, BIT2, BIT1, BIT0},      // order is 5, 6, 7, 8
    .col_pins = {BIT4, BIT5, BIT2, BIT0},    // order is 1, 2, 3, 4
    .passkey = {'1','1','1','1'},
};

char pk_attempt[4] = {'x','x','x','x'};

/**
* inits pattern transmit
*/
void trapdoor_trigger()
{
    trapdoor_wait_flag = 1;
    UCB0TBCNT = 1;
    UCB0I2CSA = TRAPDOOR_ADDR;                            
    while (UCB0CTLW0 & UCTXSTP);                      // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR | UCTXSTT;                      // I2C TX, start condition

    __delay_cycles(1000);
    current_state = GOODBYE;
    transmit_to_lcd();
}

/**
* inits lcd transmit
*/
void transmit_to_lcd()
{
    UCB0TBCNT = 1;
    UCB0I2CSA = LCD_ADDR;
    while (UCB0CTLW0 & UCTXSTP);                      // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR | UCTXSTT;                      // I2C TX, start condition
}

/**
* initializes LED 1, Timers, and LED bar ports
* 
* @param: NA
*
* @return: NA
*/
void init(void)
{
    // Disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

//------------- Setup Ports --------------------
    // LED1
    P1DIR |= BIT0;              // Config as Output
    P1OUT |= BIT0;              // turn on to start
    // LED2 and 3
    P6DIR |= BIT6 + BIT1;              
    P6OUT |= BIT6;              
    P6OUT &= ~BIT1;

    // Button
    P4DIR &= ~BIT0;         // clear 6.0 direction = input
    P4REN |= BIT0;          // enable pull-up/down resistor
    P4OUT |= BIT0;          // make resistor a pull-up

    P4IFG &= ~BIT0;          // Clear flag
    P4IE &= ~BIT0;          // disable S1 IRQ
    P4IES |= BIT0;          // Hi-low interrupt

    // Timer B0
    // Math: 1s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 8
    TB0CTL |= TBCLR;            // Clear timer and dividers
    TB0CTL |= TBSSEL__SMCLK;    // Source = SMCLK
    TB0CTL |= MC__UP;           // Mode UP
    TB0CTL |= ID__8;            // divide by 8 
    TB0EX0 |= TBIDEX__5;        // divide by 5 (100)

    TB0CCR0 = 25000;

    TB0CCTL0 &= ~CCIFG;         // Clear CCR0
    TB0CCTL0 |= CCIE;           // Enable IRQ

    // Timer B1
    // Math: .5s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 4
    TB1CTL |= TBCLR;            // Clear timer and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Source = SMCLK
    TB1CTL &= ~MC__UP;          // Mode UP, off to start
    TB1CTL |= ID__4;            // divide by 4 
    TB1EX0 |= TBIDEX__5;        // divide by 5 (100)

    TB1CCR0 = 25000;

    TB1CCTL0 &= ~CCIFG;         // Clear CCR0
    TB1CCTL0 |= CCIE;           // Enable IRQ

     // Configure Pins for I2C
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2 | BIT3;                            // I2C pins

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                             // put eUSCI_B in reset state
    UCB0CTLW0 |= UCMODE_3 | UCMST;                    // I2C master mode, SMCLK
    UCB0CTLW1 |= UCASTP_2;                            // Automatic stop after hit TBCNT
    UCB0I2CSA = LCD_ADDR;                             // configure slave address
    UCB0BRW = 0x8;                                    // baudrate = SMCLK / 8
    UCB0TBCNT = 1;                                    // num bytes to send/recieve

    UCB0CTLW0 &=~ UCSWRST;                            // clear reset register
    UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE0 | UCBCNTIE;// transmit, receive, TBCNT, and NACK

    // eCOMP
    // Configure Comparator input
    P1SEL0 |= BIT1;                           // Select eCOMP input function on P1.1/C1
    P1SEL1 |= BIT1;

      // Configure reference
    PMMCTL0_H = PMMPW_H;                      // Unlock the PMM registers
    PMMCTL2 |= INTREFEN;                      // Enable internal reference
    while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles
  
    // Setup eCOMP
    CP0CTL0 = CPPSEL0;                         // Select C1 as input for V+ terminal
    CP0CTL0 |= CPNSEL1 | CPNSEL2;              // Select DAC as input for V- terminal
    CP0CTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
    CP0CTL1 |= CPIIE | CPIE;                   // Enable eCOMP dual edge interrupt
    CP0DACCTL |= CPDACREFS | CPDACEN;          // Select on-chip VREF and enable DAC
    CP0DACDATA |= 0x003f;                      // CPDACBUF1=On-chip VREF *63/64
    CP0CTL1 |= CPEN;                           // Turn on eCOMP, in high speed mode

//------------- END PORT SETUP -------------------

    __enable_interrupt();   // enable maskable IRQs
    PM5CTL0 &= ~LOCKLPM5;   // turn on GPIO
}


/**
* Handle character reading and set base multiplier, setting patterns as well
* 
* @param: NA
*
* @return: NA
*/
int main(void)
{
    cur_char = 'Z';
    int ret = FAILURE;
    int count = 0;

    init();
    init_keypad(&keypad);

    while(1)
    {
        ret = scan_keypad(&keypad, &cur_char);
        if (ret == SUCCESS)
        {
            if(button_pressed == 1)
            {
                code_wait_flag = 1;
                current_state = REAL_ENTER;
                transmit_to_lcd();
                if (keypad.lock_state == LOCKED)  // if we're locked and trying to unlock
                {
                    reset_counter = 0;
                    pk_attempt[count] = cur_char;
                    count++;
                    if (count == 4)
                    {
                        if(check_status(&keypad, pk_attempt) == SUCCESS)
                        {
                            code_wait_flag = 0;
                            code_wait_count = 0;
                            current_state = WELCOME;
                            transmit_to_lcd();
                            TB1CTL |= MC__UP;                   // turn LED flashing on
                            trapdoor_wait_flag = 1;             // wait for 10 sec to reset
                        }
                        count = 0;
                    }
                }
            }
            else
            {
                trapdoor_trigger();
            }
            
            
        }

        if(ir_flag == 1)
        {
            current_state = START;
            transmit_to_lcd();
            ir_flag = 0;
            CPCTL1 &= ~CPEN;                           // Turn off eCOMP
            P6OUT |= BIT1;                            // turn on ready LED
            P4IE |= BIT0;                             // Enable S1 IRQ
        }

        if(trapdoor_wait_flag == 2)
        {
            P6OUT &= ~BIT1;
            trapdoor_wait_flag = 0;
            trapdoor_wait_count = 0;
            ir_flag = 0;
            button_pressed = 0;
            current_state = BLANK;
            transmit_to_lcd();
            CPCTL1 |= CPEN;                           // Turn on eCOMP, in high speed mode
        }
        if(code_wait_flag == 2)
        {
            code_wait_flag = 0;
            code_wait_count = 0;
            trapdoor_trigger();
        }


        __delay_cycles(100000);             // Delay for 100000*(1/MCLK)=0.1s
    }

    return(0);
}


//-- Interrupt Service Routines -----------------------

/**
* transmit_data
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

/**
* status_LED
*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void status_LED(void)
{
    P1OUT ^= BIT0;
    if(trapdoor_wait_flag)
    {
        trapdoor_wait_count++;
        if(trapdoor_wait_count == 10)
        {
            trapdoor_wait_flag = 2;
        }
    }
    else if(code_wait_flag)
    {
        code_wait_count++;
        if(code_wait_count == 10)
        {
            code_wait_flag++;
        }
    }
    TB0CCTL1 &= ~CCIFG;     // clear flag
}

/**
* flash LED if successfully unlocked
*/
#pragma vector = TIMER1_B0_VECTOR
__interrupt void read_temps(void)
{
    P6OUT ^= BIT6;
    TB1CCTL1 &= ~CCIFG;     // clear flag
}

/**
* Read IR sensor output and act based on changes
*/
#pragma vector = ECOMP0_ECOMP1_VECTOR
__interrupt void ECOMP0_ISR(void)
{
    switch(__even_in_range(CP0IV, CPIV__CPIIFG))
    {
        case CPIV__NONE:
            break;
        case CPIV__CPIFG:
            break;
        case CPIV__CPIIFG:          // falling edge
            if(ir_flag == 0)
            {
                ir_flag = 1;
                button_pressed = 0;
            }
            break;
        default:
            break;
    }
}

/**
* Checks to see if the button has been pressed
*/
#pragma vector = PORT4_VECTOR
__interrupt void button_press_handler(void)
{
    button_pressed = 1;
    P6OUT ^= BIT6;
    P4IE &= ~BIT0;           // disable S1 IRQ
    P4IFG &= ~BIT0;
}