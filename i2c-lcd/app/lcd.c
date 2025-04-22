/**
* @file
* @brief LCD interface functionality
*
*/
#include "src/lcd.h"
#include "intrinsics.h"
#include "msp430fr2310.h"

char *lcd_strings[] = {
    "Hello there", "Enter code", "Nice, enter code", "Welcome!",
    "Goodbye :("
};

void init_lcd(){
    P1DIR |= BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7;       // EN, RS, DB4, DB5, DB6, DB7
    P1OUT &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);    // Clear outputs

    state_flag = 0;
    current_temp_digit = 0;
    current_n = '3';
    current_pattern = 8;                      // "none"
    temp_unit = 'C';
    change_allowed = 0;

    PM5CTL0 &= ~LOCKLPM5;
    
    __delay_cycles(50000);
    lcd_set_function();
    DELAY_001;
    lcd_send_command(LCD_DISPLAY_ON);
    DELAY_001;
    lcd_send_command(LCD_ENTRY_MODE_SET);
    DELAY_001;
    lcd_send_command(LCD_CURSOR_ON);
    DELAY_001;
    lcd_send_command(LCD_CURSOR_RIGHT);
    DELAY_001;
    lcd_send_command(LCD_CLEAR_DISPLAY);
}

void lcd_send(uint8_t data, uint8_t is_data){
    if (is_data)
        P1OUT |= BIT1;        // RS = 1 for data
    else
        P1OUT &= ~BIT1;       // RS = 0 for command

    P1OUT = (data & 0xF0);    // Send high nibble (on P1.4-7)
    P1OUT |= BIT0;            // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    DELAY_0001;
    P1OUT = ((data << 4) & 0xF0);
    if (is_data) P1OUT |= BIT1;   // Re-set RS again if needed (optional)
    P1OUT |= BIT0;
    DELAY_0001;
    P1OUT &= ~BIT0;
}

void lcd_send_string(char *str){
    while (*str != '\0'){
        lcd_send_data(*str++);
    }
}

void lcd_choose_string(uint8_t data) 
{   
    lcd_clear_line(LCD_RETURN_HOME);
    DELAY_0001;
    lcd_send_string(lcd_strings[data]);
}


void lcd_set_function(){
    P1OUT &= ~BIT1;                 // RS = 0 for command
    P1OUT = (0x03 << 4);            // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x03 << 4);            // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x03 << 4);          // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x02 << 4);    // Send low nibble
    P1OUT |= BIT0;
    DELAY_0001;
    P1OUT &= ~BIT0;

    lcd_send_command(LCD_FUNCTION);
}

void lcd_clear_line(uint8_t cmd)
{
    lcd_send_command(cmd);
    DELAY_0001;
    uint8_t i = 0;
    for (i = 0; i < 16; i++) {
        lcd_send_data(' ');
        DELAY_0001;
    }
    lcd_send_command(cmd);
}
