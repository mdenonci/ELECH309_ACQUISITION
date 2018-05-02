
#ifndef UART_H
#define	UART_H

#include <xc.h> // include processor files - each processor file is guarded.  

/*Initialisation de l'UART*/
void uartInit(void);

/*Vérifie que l'envoi de donnée est possible*/
int clearToSend(void);

/*Envoie les données introduites*/
void uartSend(int txdata);

#endif	/* XC_HEADER_TEMPLATE_H */

