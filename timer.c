#include <xc.h>

void timerInit(void){
    /*
     * TIMER2 - 100 ms - P�riode Symbole
     * TIMER3 - 0.000067 ms - Fr�quence d'�chantillonnage
     * 
     */
    T2CONbits.TCKPS=0b10; //Prescaler de 64
    T4CONbits.TCKPS=0b10;
    PR2 = 62500; //Timer de p�riode 100ms --> 1 p�riode du signal re�u
    PR3 = 2667; //Timer pour ADC - 0.000067 ms (15 kHz)
    PR4 = 625; //Timer pour chaque p�riode � tester
    
    T2CONbits.TON = 1;
    T3CONbits.TON = 1; //Active les timer
    T4CONbits.TON = 1;
}