/********************************************************************************
; UFSC- Universidade Federal de Santa Catarina
; Projeto: Cafeteira Programável
; Autor: Bernardo Pandolfi Costa
; Cafeteira
;********************************************************************************/

    

#include <xc.h>
#include <pic16f877a.h>
#include <stdio.h>

#pragma config FOSC = EXTRC             // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF               // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON               // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON               // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF                // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF                // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF                // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF                 // Flash Program Memory Code Protection bit (Code protection off)

// INPUTS:
#define ST PORTBbits.RB0                // INTERRUPÇÃO
#define HOTCOLD PORTBbits.RB1           // INPUT CONFIGURAÇÃO QUENTE/GELADO
#define MILK PORTBbits.RB2              // INPUT CONFIGURAÇÃO COM LEITE OU SEM
#define WATER_VOLUME PORTBbits.RB3      // SENSOR DE VOLUME DE ÁGUA
#define MILK_VOLUME PORTBbits.RB4       // SENSOR DE VOLUME DE LEITE
#define COFFEE_WEIGHT PORTBbits.RB5     // SENSOR DE PESO DE CAFÉ
#define TEMPERATURE PORTBbits.RB6       // SENSOR DE TEMPERATURA
#define START_BTN PORTBbits.RB7         // START BUTTON

// OUTPUTS:
#define TEMP_RESISTOR PORTCbits.RC0     // RESISTÊNCIA QUE ESQUENTA A MÁQUINA
#define COFFEE_DISPOSER PORTCbits.RC1   // BOBINA QUE DESPEJA O CAFÉ
#define MILK_DISPOSER PORTCbits.RC2     // BOBINA QUE DESPEJA O LEITE
#define COFFEE_DONE PORTCbits.RC3       // INDICADOR DE CAFÉ PRONTO

// LCD DISPLAY:
#define RS PORTDbits.RD0
#define RW PORTDbits.RD1
#define EN PORTDbits.RD2
#define D4 PORTDbits.RD4
#define D5 PORTDbits.RD5
#define D6 PORTDbits.RD6
#define D7 PORTDbits.RD7


#define _XTAL_FREQ 4000000

#include "lcd.h"

int main()
{
  char buffer[20];
  
  TRISD = 0x00;  //configura PORTD como sa?da. Local onde deve estar o LCD
  TRISC = 0x00;
  TEMP_RESISTOR = 0;
  COFFEE_DISPOSER = 0;
  MILK_DISPOSER = 0;
  COFFEE_DONE = 0;
  
  Lcd_Init();    //necess?rio para o LCD iniciar
  
  while(1)
  {
    if(WATER_VOLUME == 1 && COFFEE_WEIGHT == 0) {
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("NO WATER LEFT!! ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
    }
    else if(WATER_VOLUME == 0 && COFFEE_WEIGHT == 1) {
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("NO COFFEE LEFT!!");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
    }
    else if(WATER_VOLUME == 1 && COFFEE_WEIGHT == 1) {
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("NO WATER LEFT!! ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("NO COFFEE LEFT!!");
    }
    else if(MILK_VOLUME == 1 && MILK == 1) {
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("NO MILK LEFT!!  ");
    }
    else {
        Lcd_Set_Cursor(1,1);            //P?e curso linha 1 coluna 1
        Lcd_Write_String("Your coffee:    ");  //escreve string   
        Lcd_Set_Cursor(2,1);             //linha 2 coluna 1

        if(HOTCOLD == 1 && MILK == 1 && MILK_VOLUME == 0) {
            Lcd_Write_String("Cold w/ Milk    "); //escreve string
        }
        else if(HOTCOLD == 1 && MILK == 0) {
            Lcd_Write_String("Cold wo/ Milk   "); //escreve string
        }
        else if (HOTCOLD == 0 && MILK == 1  && MILK_VOLUME == 0){
            Lcd_Write_String("Hot w/ Milk     "); //escreve string
        }
        else if (HOTCOLD == 0 && MILK == 0) {
            Lcd_Write_String("Hot wo/ Milk    "); //escreve string
        }
        
        if(START_BTN == 1) {
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("Preparing Coffee");
            if(HOTCOLD == 0) {
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("Warming Up...   ");
                while(TEMPERATURE == 0){
                    TEMP_RESISTOR = 1;
                };
                TEMP_RESISTOR = 0;
            }
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("Pouring Coffee..");
            COFFEE_DISPOSER = 1;
            __delay_ms(3000);
            COFFEE_DISPOSER = 0;
            if(MILK == 1) {
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("Pouring Milk... ");
                MILK_DISPOSER = 1;
                __delay_ms(3000);
                MILK_DISPOSER = 0;
            }
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            COFFEE_DONE = 1;
            Lcd_Write_String("Coffee Done!!!  ");
            __delay_ms(3000);
            COFFEE_DONE = 0;
        }
    }
    
    
    
    
    
  }
  return 0;
}