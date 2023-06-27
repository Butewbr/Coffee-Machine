/********************************************************************************
; UFSC- Universidade Federal de Santa Catarina
; Projeto: Cafeteira Program�vel
; Autor: Bernardo Pandolfi Costa
; Data: 28 de Junho de 2023
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
#define ST PORTBbits.RB0                // INTERRUP��O
#define HOTCOLD PORTBbits.RB1           // INPUT CONFIGURA��O QUENTE/GELADO
#define MILK PORTBbits.RB2              // INPUT CONFIGURA��O COM LEITE OU SEM
#define WATER_VOLUME PORTBbits.RB3      // SENSOR DE VOLUME DE �GUA
#define MILK_VOLUME PORTBbits.RB4       // SENSOR DE VOLUME DE LEITE
#define COFFEE_WEIGHT PORTBbits.RB5     // SENSOR DE PESO DE CAF�
#define TEMPERATURE PORTBbits.RB6       // SENSOR DE TEMPERATURA
#define START_BTN PORTBbits.RB7         // START BUTTON

// OUTPUTS:
#define TEMP_RESISTOR PORTCbits.RC0     // RESIST�NCIA QUE ESQUENTA A M�QUINA
#define COFFEE_DISPOSER PORTCbits.RC1   // BOBINA QUE DESPEJA O CAF�
#define MILK_DISPOSER PORTCbits.RC2     // BOBINA QUE DESPEJA O LEITE
#define COFFEE_DONE PORTCbits.RC3       // INDICADOR DE CAF� PRONTO

// LCD DISPLAY:
#define RS PORTDbits.RD0                // DEFINE RS DO LCD
#define RW PORTDbits.RD1                // DEFINE RW DO LCD
#define EN PORTDbits.RD2                // DEFINE EN DO LCD
#define D4 PORTDbits.RD4                // DEFINE D4 NO LCD
#define D5 PORTDbits.RD5                // DEFINE D5 NO LCD
#define D6 PORTDbits.RD6                // DEFINE D6 NO LCD
#define D7 PORTDbits.RD7                // DEFINE D7 NO LCD

#define _XTAL_FREQ 4000000              // DEFINE FREQU�NCIA

#include "lcd.h"                        // incluir configura��es do LCD

int conta = 0; // vari�vel auxiliar pra contar o tempo
int coffee_or_milk = 0; // vari�vel auxiliar para o timer; se vai chamar a interrup��o de despejar o caf� (0) ou leite (1)
int milk_flag = 1; // flag que indica se o leite j� foi despejado (padr�o em 1, porque pode ser que o usu�rio n�o queira leite)
int coffee_flag = 0; // flag que indica que o caf� j� foi despejado
int preparating = 0; // flag que indica se o caf� est� sendo preparado no momento

void __interrupt() TrataInt(void) { // fun��o de interrup��o
    if (TMR1IF) // verifica se foi a interrup��o da contagem
    {
        PIR1bits.TMR1IF = 0; // reseta o flag da interrup��o
        TMR1L = 0xDC; // reinicia os valores da contagem
        TMR1H = 0x0B; // reinicia os valores da contagem

        conta++; // incrementa a contagem
        if (conta == 10) { // verifica se j� passou o tempo

            if (coffee_or_milk == 0) { // verifica se � o timer � de despejar caf�
                COFFEE_DONE = 1; // liga o indicador de caf� pronto
                coffee_flag = 1; // ativa a flag que o caf� j� foi despejado
            } else if (coffee_or_milk == 1) { // verifica se o timer � de despejar o leite 
                milk_flag = 1; // indica que o leite j� foi despejado
            } else { // se n�o for nem de caf� nem de leite, � um timer para o indicador de caf� pronto
                COFFEE_DONE = 0; // desliga o indicador de caf� pronto
            }

            conta = 0; // reinicia a conta
        }
    }
    if (INTF) // verifica se foi a interrup��o externa
    {
        INTCONbits.INTF = 0; // reseta o flag da interrup��o

        if (preparating == 1) { // verifica se o caf� est� sendo preparado pra cancelar algo
            COFFEE_DISPOSER = 0; // desliga a v�lvula do caf�
            MILK_DISPOSER = 0; // desliga a v�lvula do leite
            coffee_flag = 1; // considera caf� despejado
            milk_flag = 1; // considera leite despejado
            TEMP_RESISTOR = 0;
        }

    }
    return; // retorna ao programa
}

int main() { // fun��o main do programa
    OPTION_REGbits.nRBPU = 0; // ativa resistores de pull-ups

    TRISD = 0x00; // configura PORTD como sa�da (LCD)
    TRISC = 0x00; // configura PORTC como sa�da (demais outputs)
    TEMP_RESISTOR = 0; // inicia outputs como 0
    COFFEE_DISPOSER = 0; // inicia outputs como 0
    MILK_DISPOSER = 0; // inicia outputs como 0
    COFFEE_DONE = 0; // inicia outputs como 0

    INTCONbits.GIE = 1; // habilita int global
    INTCONbits.PEIE = 1; // habilita int dos perif�ricos
    PIE1bits.TMR1IE = 1; // habilitar int do timer 1

    T1CONbits.TMR1CS = 0; // define timer 1 como temporizador
    T1CONbits.T1CKPS0 = 1; // bit para configurar pre-escaler, neste caso 1:8
    T1CONbits.T1CKPS1 = 1; // bit para configurar pre-escaler, neste caso 1:8

    TMR1L = 0xDC; // carga do valor inicial do contador
    TMR1H = 0x0B; // quando estourar passou 0.5s

    Lcd_Init(); //necess�rio para o LCD iniciar

    while (1) { // loop do programa
        coffee_flag = 0; // coloca a flag de caf� despejado em 0
        milk_flag = 0; // coloca a flag de leite despejado em 0
        if (WATER_VOLUME == 1 && COFFEE_WEIGHT == 0) { // se t� faltando �gua, mas tem caf�
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO WATER LEFT!! "); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("                "); // apaga apenas a segunda linha
        } else if (WATER_VOLUME == 0 && COFFEE_WEIGHT == 1) { // verifica se t� faltando caf� e �gua n�o
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO COFFEE LEFT!!"); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("                "); // apaga apenas a segunda linha
        } else if (WATER_VOLUME == 1 && COFFEE_WEIGHT == 1) { // verifica se t� faltando caf� e �gua
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO WATER LEFT!! "); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("NO COFFEE LEFT!!"); // escreve na tela
        } else if (MILK_VOLUME == 1 && MILK == 1) { // verifica se t� faltando caf� e o usu�rio t� querendo caf�
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO MILK LEFT!!  "); // escreve na tela
        } else { // caso contr�rio escreve a mensagem padr�o
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("Your coffee:    "); //escreve string   
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha

            if (HOTCOLD == 1 && MILK == 1 && MILK_VOLUME == 0) { // verifica se o usu�rio quer um caf� gelado e com leite
                Lcd_Write_String("Cold w/ Milk    "); //escreve string
            } else if (HOTCOLD == 1 && MILK == 0) { // verifica se o usu�rio quer um caf� gelado sem leite
                Lcd_Write_String("Cold wo/ Milk   "); //escreve string
            } else if (HOTCOLD == 0 && MILK == 1 && MILK_VOLUME == 0) { // verifica se o usu�rio quer um caf� com leite quente
                Lcd_Write_String("Hot w/ Milk     "); //escreve string
            } else if (HOTCOLD == 0 && MILK == 0) { // verifica se o usu�rio quer um caf� preto quente
                Lcd_Write_String("Hot wo/ Milk    "); //escreve string
            }

            if (START_BTN == 1) { // verifica se o usu�rio apertou o bot�o de iniciar
                T1CONbits.TMR1ON = 0; // desativa o timer por enquanto
                preparating = 1; // ativa a flag de que o caf� est� sendo preparado
                Lcd_Clear(); // limpa a tela do LCD
                Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
                Lcd_Write_String("Preparing Coffee"); // escreve na tela
                if (HOTCOLD == 0) { // verifica se o usu�rio configurou que quer quente
                    Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                    Lcd_Write_String("Warming Up...   "); // escreve na tela
                    while (TEMPERATURE == 0) { // enquanto o sensor de temperatura n�o indicar que est� quente
                        TEMP_RESISTOR = 1; // o resistor para esquentar ficar� ligado
                    }
                    TEMP_RESISTOR = 0; // ao fim, desliga-se o resistor
                }
                T1CONbits.TMR1ON = 1; // liga o timer
                if (MILK == 1) { // verifica se o usu�rio configurou com leite
                    milk_flag = 0; // coloca a flag de leite despejado em 0
                    Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                    Lcd_Write_String("Pouring Milk... "); // escreve na tela
                    
                    while (milk_flag == 0) { // enquanto o leite n�o for despejado
                        MILK_DISPOSER = 1; // deixa a v�lvula de leite ligada
                        coffee_or_milk = 1; // coloca a vari�vel auxiliar para indicar que leite est� sendo despejado
                    }
                    MILK_DISPOSER = 0; // desliga a v�lvula do leite
                }
                
                Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                Lcd_Write_String("Pouring Coffee.."); // escreve na tela
                while (coffee_flag == 0) { // enquanto o caf� n�o for despejado
                    COFFEE_DISPOSER = 1; // deixa a v�lvula de caf� ligada
                    coffee_or_milk = 0; // coloca a vari�vel auxiliar para indicar que caf� est� sendo despejado
                }
                COFFEE_DISPOSER = 0; // desliga a v�lvula de caf�
                coffee_or_milk = 2; // coloca a vari�vel auxiliar para indicar que nem caf� nem leite est�o despejados, mas sim que o caf� est� pronto
            }
        }
    }
    return 0; // finaliza o programa (nunca)
}