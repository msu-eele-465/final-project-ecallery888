
//******************************************************************************
// MSP430FR235x Demo - eCOMP output toggle, wake up from LPM3;
//                     Vcompare is compared against 1V.
//
// Description: Use eCOMP and internal VREF to determine if input 'Vcompare'
//    is high or low.  When Vcompare exceeds 1V(VREF*43/64), CPOUT goes high
//    and LED is turned on. When Vcompare is less than 1V(VREF*43/64) then
//    CPOUT goes low and LED is turned off. eCOMP interrupt enabled, wake up
//    from LPM3.
//
//                MSP430FR2355
//             ------------------
//         /|\|                  |
//          | |                  |
//          --|RST        P1.1/C1|<--Vcompare
//            |         P2.0/COUT|--> 'high'(Vcompare>1V); 'low'(Vcompare<1V)
//            |              P1.2|--> LED
//            |                  |
//
//   Cash Hao
//   Texas Instruments Inc.
//   November 2016
//   Built with IAR Embedded Workbench v6.50.0 & Code Composer Studio v6.2.0
//******************************************************************************
#include <msp430.h>

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

  // Configure GPIO
  P1OUT &= ~BIT0;                           // Clear P1.2 output latch for a defined power-on state
  P1DIR |= BIT0;                            // Set P1.2 to output direction

  // Configure Comparator input & output
//   P1SEL0 |= BIT1;                           // Select eCOMP input function on P1.1/C1
//   P1SEL1 |= BIT1;
// //   P2DIR |= BIT0;
// //   P2SEL1 |= BIT0;                           // Select CPOUT function on P2.0/COUT

//   PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
//                                             // to activate previously configured port settings

//   // Configure reference module
//   PMMCTL0_H = PMMPW_H;                    // Unlock the PMM registers
//   PMMCTL2 = INTREFEN | REFVSEL_0;         // Enable internal 1.5V reference
//   while(!(PMMCTL2 & REFGENRDY));          // Poll till internal reference settles

//   // Setup eCOMP
//   CP0CTL0 = CPPSEL0;                         // Select C1 as input for V+ terminal
//   CP0CTL0 |= CPNSEL1 | CPNSEL2;              // Select DAC as input for V- terminal
//   CP0CTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
//   CP0CTL1 |= CPIIE | CPIE;                   // Enable eCOMP dual edge interrupt
//   CP0DACCTL |= CPDACREFS | CPDACEN;          // Select on-chip VREF and enable DAC
//   CP0DACDATA |= 0x002B;                      // CPDACBUF1=On-chip VREF*43/64 = 1V
//   CP0CTL1 |= CPEN;                           // Turn on eCOMP, in high speed mode
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


//   __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3
  __enable_interrupt();   // enable maskable IRQs 
  __no_operation();                         // For debug
}

// eCOMP interrupt service routine
#pragma vector = ECOMP0_ECOMP1_VECTOR
__interrupt void ECOMP0_ISR(void)
{
    switch(__even_in_range(CP0IV, CPIV__CPIIFG))
    {
        case CPIV__NONE:
            break;
        case CPIV__CPIFG:
            P1OUT |= BIT0;
            break;
        case CPIV__CPIIFG:
            P1OUT &= ~BIT0;
            break;
        default:
            break;
    }
}
