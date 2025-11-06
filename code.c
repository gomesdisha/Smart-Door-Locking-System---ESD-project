#include <lpc17xx.h>
#include <stdio.h>
#include <string.h>

#define RS_CTRL 0x08000000 // P0.27
#define EN_CTRL 0x10000000 // P0.28
#define DT_CTRL 0x07800000 // P0.23 to P0.26
#define PIN_LENGTH 4

unsigned long temp=0, temp11=0, temp12=0, i;
unsigned char flag=0, flag1=0, flag2=0;
unsigned char col, row, j=3, attempts_left=3;
unsigned char key[4][4] = {{'0','1','2','3'},
                          {'4','5','6','7'},
                          {'8','9','A','B'},
                          {'C','D','E','F'}};
const char correct_pin[PIN_LENGTH] = {'1','2','3','4'}; // Change your PIN here
unsigned char input_pin[PIN_LENGTH];
unsigned char pin_index = 0;

unsigned char seven_seg[4]={0x3F,0x06,0x5B,0x4F}; // 3,2,1,0
unsigned long init_command[] = {0x33,0x32,0x28,0x0C,0x06,0x01,0x80};

void lcd_write(void);
void port_write(void);
void scan(void);
void delay_lcd(unsigned int);
void counter(void);
void delay_ms(unsigned int);
void lcd_clear(void);
void lcd_print(char *str);
void system_lockout(void);

int main() {
    SystemInit();
    SystemCoreClockUpdate();
   
    // GPIO Configuration
    LPC_GPIO0->FIODIR |= 0x00078FF0; //P0.4-0.11 and P0.15-P.18
    LPC_GPIO0->FIODIR |= DT_CTRL | RS_CTRL | EN_CTRL;
    LPC_GPIO2->FIODIR |= 0x00003C00; // Rows P2.10-P2.13
    LPC_GPIO1->FIODIR &= ~(0x07800000); // Columns P1.23-P1.26
   
    // LCD Initialization
    for(i=0; i<7; i++) {
        temp11 = init_command[i];
        lcd_write();
    }
lcd_clear();
    lcd_print("Enter PIN:");

   
    while(1) {
        for(row=0; row<4; row++) {
            LPC_GPIO2->FIOPIN = (1 << (row + 10));
            scan();
           
            if(flag && pin_index < PIN_LENGTH) {
                input_pin[pin_index++] = key[row][col];
                temp11 = '*';
                flag1 = 1;
                lcd_write();
                delay_ms(200);
            }
           
            if(pin_index == PIN_LENGTH) {
                if(memcmp(input_pin, correct_pin, PIN_LENGTH) == 0) {
                    lcd_clear();
                    lcd_print("Door Open");
                    LPC_GPIO0->FIOSET = (1<<19); // Buzzer (if connected)
                    delay_ms(2000);
                    LPC_GPIO0->FIOCLR = (1<<19);
                    pin_index = 0;
                    attempts_left = 3;
                    lcd_clear();
                    lcd_print("Enter PIN:");
                }
                else {
                    attempts_left--;
                    lcd_clear();
                    lcd_print("Incorrect!");
                    counter();
                   
                    if(attempts_left == 0) {
                        system_lockout();
                        attempts_left = 3;
                    }
                   
                    pin_index = 0;
                    delay_ms(1000);
                    lcd_clear();
                    lcd_print("Enter PIN:");
                }
            }
        }
    }
}

void scan() {
    unsigned long temp3 = LPC_GPIO1->FIOPIN & 0x07800000;
    flag = 0;
   
    if(temp3) {
        delay_ms(20); // Debounce
        temp3 = LPC_GPIO1->FIOPIN & 0x07800000;
        if(temp3) {
            flag = 1;
            if(temp3 & (1<<23)) col = 0;
            else if(temp3 & (1<<24)) col = 1;
            else if(temp3 & (1<<25)) col = 2;
            else if(temp3 & (1<<26)) col = 3;
        }
    }
}

void counter() {

        LPC_GPIO0->FIOPIN = seven_seg[attempts_left] << 4;
        delay_ms(1000);

}

void system_lockout() {
    lcd_clear();
    lcd_print("Device Locked");
    for(i=60; i>0; i--) {
        LPC_GPIO0->FIOPIN = seven_seg[0] << 4; // Display 0
        delay_ms(1000);
    }
    lcd_clear();
    lcd_print("Enter PIN:");
}

void lcd_print(char *str) {
    flag1 = 1;
    while(*str) {
        temp11 = *str++;
        lcd_write();
    }
}

void lcd_clear() {
    flag1 = 0;
    temp11 = 0x01;
    lcd_write();
    delay_ms(100);
}

void lcd_write() {
    temp12 = temp11 & 0xF0;
    temp12 = temp12 << 19;
    port_write();
    temp12 = (temp11 & 0x0F) << 23;
    port_write();
}

void port_write() {
    LPC_GPIO0->FIOPIN = temp12;
    LPC_GPIO0->FIOCLR = RS_CTRL | EN_CTRL;
    if(flag1) LPC_GPIO0->FIOSET = RS_CTRL;
    LPC_GPIO0->FIOSET = EN_CTRL;
    delay_lcd(500);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(500);
}

void delay_ms(unsigned int ms) {
unsigned int o,j;
    for(o=0; o<ms; o++){
        for(j=0; j<20000; j++);}
}

void delay_lcd(unsigned int r1) {
    while(r1--) __NOP();
}
