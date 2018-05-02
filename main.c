// BREADBOARD

#include "init.h"
#include "uart.h"
#include "timer.h"
#include "decoder.h"
#include "FskDetector.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include <math.h>
#include <p33FJ128GP802.h>



int main(void)
{
    /*
     * =========================
     * Initialisations 
     * ========================
     */
    
    int message;
    oscillatorInit();
    timerInit();
    adcTimerInit();
    
    /*
     * =========================
     * LOOP
     * ========================
     */
    
	while(1) {
        
        /* 1 - Echantillonnage et Décodage */
        
        message = decode100ms();
        
        /* 2 - Envoi par UART */
        
        if (clearToSend()){
            uartSend(message);
        }
    }
}