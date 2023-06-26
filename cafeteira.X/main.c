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
#define RS PORTDbits.RD0                // DEFINE RS DO LCD
#define RW PORTDbits.RD1                // DEFINE RW DO LCD
#define EN PORTDbits.RD2                // DEFINE EN DO LCD
#define D4 PORTDbits.RD4                // DEFINE D4 NO LCD
#define D5 PORTDbits.RD5                // DEFINE D5 NO LCD
#define D6 PORTDbits.RD6                // DEFINE D6 NO LCD
#define D7 PORTDbits.RD7                // DEFINE D7 NO LCD

#define _XTAL_FREQ 4000000              // DEFINE FREQUÊNCIA

#include "lcd.h"                        // incluir configurações do LCD

int conta=0;                            // variável auxiliar pra contar o tempo

void __interrupt() TrataInt(void)

{    
  if (TMR1IF)                           // verifica se foi a interrupção da contagem
     {  
        PIR1bits.TMR1IF = 0;            // reseta o flag da interrupção
        TMR1L = 0xDC;                   // reinicia os valores da contagem
        TMR1H = 0x0B;                   // reinicia os valores da contagem
        
        conta++;                        // incrementa a contagem
        if (conta==10) {                // verifica se já passaram os 3 segundos

            //TRATAR AQUI
            
            conta = 0;                  // reinicia a conta
        }
  }
  return;                               // retorna ao programa
}

int main()
{
  OPTION_REGbits.nRBPU = 0;             // ativa resistores de pull-ups
  
  TRISD = 0x00;                         // configura PORTD como saída (LCD)
  TRISC = 0x00;                         // configura PORTC como saída (demais outputs)
  TEMP_RESISTOR = 0;                    // inicia outputs como 0
  COFFEE_DISPOSER = 0;                  // inicia outputs como 0
  MILK_DISPOSER = 0;                    // inicia outputs como 0
  COFFEE_DONE = 0;                      // inicia outputs como 0
  
  INTCONbits.GIE = 1;                   // habilita int global
  INTCONbits.PEIE = 1;                  // habilita int dos periféricos
  PIE1bits.TMR1IE = 1;                  // habilitar int do timer 1
  
  T1CONbits.TMR1CS = 0;                 // define timer 1 como temporizador
  T1CONbits.T1CKPS0 = 1;                // bit para configurar pre-escaler, neste caso 1:8
  T1CONbits.T1CKPS1 = 1;                // 
  
  TMR1L = 0xDC;                         // carga do valor inicial do contador
  TMR1H = 0x0B;                         // quando estourar passou 0.5s
  
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