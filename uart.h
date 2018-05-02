
#ifndef UART_H
#define	UART_H

#include <xc.h> // include processor files - each processor file is guarded.  

/*Initialisation de l'UART*/
void uartInit(void);

/*V�rifie que l'envoi de donn�e est possible*/
int clearToSend(void);

/*Envoie les donn�es introduites*/
void uartSend(int txdata);

#endif	/* XC_HEADER_TEMPLATE_H */

