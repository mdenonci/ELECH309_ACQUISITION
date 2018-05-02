#include <xc.h>

void uartInit(void){
    RPOR1bits.RP2R = 0b00011; // RP2 est associé à UART TX 1
    U1BRG = 86; //115200
    U1MODEbits.BRGH = 0;
    U1MODEbits.PDSEL = 0b00;
    U1MODEbits.STSEL = 0;
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
}

int clearToSend(void){
    //Check if buffer is full
    if (!U1STAbits.UTXBF){
        return 1;
    }else{
        return 0;
    }
}

void uartSend(int txdata){
    U1TXREG = txdata;
}