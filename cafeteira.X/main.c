/********************************************************************************
; UFSC- Universidade Federal de Santa Catarina
; Projeto: Cafeteira Programável
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

int conta = 0; // variável auxiliar pra contar o tempo
int coffee_or_milk = 0; // variável auxiliar para o timer; se vai chamar a interrupção de despejar o café (0) ou leite (1)
int milk_flag = 1; // flag que indica se o leite já foi despejado (padrão em 1, porque pode ser que o usuário não queira leite)
int coffee_flag = 0; // flag que indica que o café já foi despejado
int preparating = 0; // flag que indica se o café está sendo preparado no momento

void __interrupt() TrataInt(void) { // função de interrupção
    if (TMR1IF) // verifica se foi a interrupção da contagem
    {
        PIR1bits.TMR1IF = 0; // reseta o flag da interrupção
        TMR1L = 0xDC; // reinicia os valores da contagem
        TMR1H = 0x0B; // reinicia os valores da contagem

        conta++; // incrementa a contagem
        if (conta == 10) { // verifica se já passou o tempo

            if (coffee_or_milk == 0) { // verifica se é o timer é de despejar café
                COFFEE_DONE = 1; // liga o indicador de café pronto
                coffee_flag = 1; // ativa a flag que o café já foi despejado
            } else if (coffee_or_milk == 1) { // verifica se o timer é de despejar o leite 
                milk_flag = 1; // indica que o leite já foi despejado
            } else { // se não for nem de café nem de leite, é um timer para o indicador de café pronto
                COFFEE_DONE = 0; // desliga o indicador de café pronto
            }

            conta = 0; // reinicia a conta
        }
    }
    if (INTF) // verifica se foi a interrupção externa
    {
        INTCONbits.INTF = 0; // reseta o flag da interrupção

        if (preparating == 1) { // verifica se o café está sendo preparado pra cancelar algo
            COFFEE_DISPOSER = 0; // desliga a válvula do café
            MILK_DISPOSER = 0; // desliga a válvula do leite
            coffee_flag = 1; // considera café despejado
            milk_flag = 1; // considera leite despejado
            TEMP_RESISTOR = 0;
        }

    }
    return; // retorna ao programa
}

int main() { // função main do programa
    OPTION_REGbits.nRBPU = 0; // ativa resistores de pull-ups

    TRISD = 0x00; // configura PORTD como saída (LCD)
    TRISC = 0x00; // configura PORTC como saída (demais outputs)
    TEMP_RESISTOR = 0; // inicia outputs como 0
    COFFEE_DISPOSER = 0; // inicia outputs como 0
    MILK_DISPOSER = 0; // inicia outputs como 0
    COFFEE_DONE = 0; // inicia outputs como 0

    INTCONbits.GIE = 1; // habilita int global
    INTCONbits.PEIE = 1; // habilita int dos periféricos
    PIE1bits.TMR1IE = 1; // habilitar int do timer 1

    T1CONbits.TMR1CS = 0; // define timer 1 como temporizador
    T1CONbits.T1CKPS0 = 1; // bit para configurar pre-escaler, neste caso 1:8
    T1CONbits.T1CKPS1 = 1; // bit para configurar pre-escaler, neste caso 1:8

    TMR1L = 0xDC; // carga do valor inicial do contador
    TMR1H = 0x0B; // quando estourar passou 0.5s

    Lcd_Init(); //necessário para o LCD iniciar

    while (1) { // loop do programa
        coffee_flag = 0; // coloca a flag de café despejado em 0
        milk_flag = 0; // coloca a flag de leite despejado em 0
        if (WATER_VOLUME == 1 && COFFEE_WEIGHT == 0) { // se tá faltando água, mas tem café
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO WATER LEFT!! "); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("                "); // apaga apenas a segunda linha
        } else if (WATER_VOLUME == 0 && COFFEE_WEIGHT == 1) { // verifica se tá faltando café e água não
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO COFFEE LEFT!!"); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("                "); // apaga apenas a segunda linha
        } else if (WATER_VOLUME == 1 && COFFEE_WEIGHT == 1) { // verifica se tá faltando café e água
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO WATER LEFT!! "); // escreve na tela
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
            Lcd_Write_String("NO COFFEE LEFT!!"); // escreve na tela
        } else if (MILK_VOLUME == 1 && MILK == 1) { // verifica se tá faltando café e o usuário tá querendo café
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("NO MILK LEFT!!  "); // escreve na tela
        } else { // caso contrário escreve a mensagem padrão
            Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
            Lcd_Write_String("Your coffee:    "); //escreve string   
            Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha

            if (HOTCOLD == 1 && MILK == 1 && MILK_VOLUME == 0) { // verifica se o usuário quer um café gelado e com leite
                Lcd_Write_String("Cold w/ Milk    "); //escreve string
            } else if (HOTCOLD == 1 && MILK == 0) { // verifica se o usuário quer um café gelado sem leite
                Lcd_Write_String("Cold wo/ Milk   "); //escreve string
            } else if (HOTCOLD == 0 && MILK == 1 && MILK_VOLUME == 0) { // verifica se o usuário quer um café com leite quente
                Lcd_Write_String("Hot w/ Milk     "); //escreve string
            } else if (HOTCOLD == 0 && MILK == 0) { // verifica se o usuário quer um café preto quente
                Lcd_Write_String("Hot wo/ Milk    "); //escreve string
            }

            if (START_BTN == 1) { // verifica se o usuário apertou o botão de iniciar
                T1CONbits.TMR1ON = 0; // desativa o timer por enquanto
                preparating = 1; // ativa a flag de que o café está sendo preparado
                Lcd_Clear(); // limpa a tela do LCD
                Lcd_Set_Cursor(1, 1); // coloca o cursor do LCD na primeira linha
                Lcd_Write_String("Preparing Coffee"); // escreve na tela
                if (HOTCOLD == 0) { // verifica se o usuário configurou que quer quente
                    Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                    Lcd_Write_String("Warming Up...   "); // escreve na tela
                    while (TEMPERATURE == 0) { // enquanto o sensor de temperatura não indicar que está quente
                        TEMP_RESISTOR = 1; // o resistor para esquentar ficará ligado
                    }
                    TEMP_RESISTOR = 0; // ao fim, desliga-se o resistor
                }
                T1CONbits.TMR1ON = 1; // liga o timer
                if (MILK == 1) { // verifica se o usuário configurou com leite
                    milk_flag = 0; // coloca a flag de leite despejado em 0
                    Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                    Lcd_Write_String("Pouring Milk... "); // escreve na tela
                    
                    while (milk_flag == 0) { // enquanto o leite não for despejado
                        MILK_DISPOSER = 1; // deixa a válvula de leite ligada
                        coffee_or_milk = 1; // coloca a variável auxiliar para indicar que leite está sendo despejado
                    }
                    MILK_DISPOSER = 0; // desliga a válvula do leite
                }
                
                Lcd_Set_Cursor(2, 1); // coloca o cursor do LCD na segunda linha
                Lcd_Write_String("Pouring Coffee.."); // escreve na tela
                while (coffee_flag == 0) { // enquanto o café não for despejado
                    COFFEE_DISPOSER = 1; // deixa a válvula de café ligada
                    coffee_or_milk = 0; // coloca a variável auxiliar para indicar que café está sendo despejado
                }
                COFFEE_DISPOSER = 0; // desliga a válvula de café
                coffee_or_milk = 2; // coloca a variável auxiliar para indicar que nem café nem leite estão despejados, mas sim que o café está pronto
            }
        }
    }
    return 0; // finaliza o programa (nunca)
}