/**
* @file
* @brief Implements LED_status and keypad to operate a pattern-displaying LED bar
*
*/
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/keypad.h"
#include "../src/led_status.h"
#include "intrinsics.h"
#include "msp430fr2355.h"


uint8_t current_state = 0, reset_counter = 0, current_state = 0;
// starting values for every pattern
const uint8_t TRAPDOOR_ADDR = 0x0A, LCD_ADDR = 0x0B;
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
void transmit_pattern()
{
    UCB0TBCNT = 1;
    UCB0I2CSA = LED_BAR_ADDR;                            
    while (UCB0CTLW0 & UCTXSTP);                      // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR | UCTXSTT;                      // I2C TX, start condition
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
    // LED2
    P6DIR |= BIT6;              // Config as Output
    P6OUT |= BIT6;              // turn on to start

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
    // Math: 1s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 4
    TB1CTL |= TBCLR;            // Clear timer and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Source = SMCLK
    TB1CTL |= MC__UP;           // Mode UP
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
    UCB0IE |= UCTXIE0 | UCNACKIE | UCBCNTIE;// transmit, receive, TBCNT, and NACK
    
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

    while(true)
    {
        ret = scan_keypad(&keypad, &cur_char);
        __delay_cycles(100000);             // Delay for 100000*(1/MCLK)=0.1s
        if (ret == SUCCESS)
        {
            if (keypad.lock_state == LOCKED)  // if we're locked and trying to unlock
            {
                reset_counter = 0;
                pk_attempt[count] = cur_char;
                count++;
                if (count == 4)
                {
                    if(check_status(&keypad, pk_attempt) == SUCCESS)
                    {
                        set_lock(&keypad, UNLOCKED);
                    }
                    count = 0;
                }
            }
            
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
            UCB0TXBUF = 0;                          
        }
        break;                                  
    default:
        break;
    }

}

/**
* read from ADC every .5s
*/
#pragma vector = TIMER1_B0_VECTOR
__interrupt void read_ADC(void)
{
    P6OUT ^= BIT6;
    adc_flag = 1;
     if (current_idx == oldest_idx) {
        // overwriting the oldest index, subtract from total before saving
        total -= temp_buffer[oldest_idx];
        oldest_idx = (oldest_idx + 1) % window_size;
    }

    prev_idx = current_idx;
    current_idx = (current_idx + 1) % window_size;

    if (!init_count) {
       avg_temp_flag = 1;
    } else if (init_count && (current_idx == (window_size - 1))){
        // wait to send average temp
        init_count = 0;
    }
    TB1CCTL1 &= ~CCIFG;     // clear flag
}

/**
* Read temperature value from ADC
*/
#pragma vector = ADC_VECTOR
__interrupt void recordAV(void)
{
    // save to current index
    temp_buffer[prev_idx] = ADCMEM0;
    total += ADCMEM0;
    
}