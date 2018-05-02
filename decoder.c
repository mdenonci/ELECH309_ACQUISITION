#include <xc.h>
#include "FskDetector.h"
#include "adc.h"

#define REG_SIZE 3
#define PI 3.14
#define STAGES 4
#define SIZESIGNAL 20
#define SAMPLES 1500
#define THRESHOLDVALUE 20
#define PERSAMPLES 15
#define NBSAMPLES (SAMPLES/PERSAMPLES)

/* DEFINITIONS DES LISTES ET VARIABLES */

volatile long FilteredSignal900[SIZESIGNAL]; //= (long * )malloc( SIZESIGNAL*sizeof(long)); // Make a List of SIZESIGNAL values (SIZESIGNAL= number of values in one period) composed by filtered values
volatile long FilteredSignal1100[SIZESIGNAL];// Make a List of SIZESIGNAL values (SIZESIGNAL= number of values in one period) composed by filtered values
volatile long Reg900[(STAGES+1)*REG_SIZE]; // Make a Register List used to filter the Signal
volatile long Reg1100[(STAGES+1)*REG_SIZE]; // Make a Register List used to filter the Signal

volatile int detLOW = 0;
volatile int detHIGH = 0;

/* FONCTIONS */

/* Ces fonctions servent � �chantillonner et d�coder le message. Le principe est
 * le suivant:
 * 
 * On �chantillonne � 15kHz un signal audio dont la p�riode symbole est de 
 * 100 ms. On a donc, par symbole, 1500 �chantillons. On va alors regrouper ces
 * 1500 �chantillons en 100 "groupes d'�chantillons" contenants chacuns 15 
 * valeurs mesur�es. Chaque groupe sera ensuite filtr� afin d'y d�tecter l'�ventuelle pr�sence
 * d'un signal � 900 ou 1100 Hz. 
 * 
 * On aura donc, pour 1 symbole de 100 ms, 100 indications sur la pr�sence 
 * de 900 Hz et 100 indications pour 1100 Hz.
 * 
 * On regarde alors ces listes afin de d�terminer, via la majorit�, si oui ou non il y'a bel
 * et bien eu du 900 Hz (1100 Hz), ce qui modifie les valeurs de isHIGH et isLOW
 * Il y'a d�s lors plusieurs possibilit� qui sont trait�es par la fonction fskDetector:
 * 
 * isHIGH = isLOW = 1 -> bruit (les deux fr�quences sont pr�sentes)
 * isHIGH = isLOW = 0 -> silence
 * isHIGH = 1 | isLOW = 0 -> 1100 Hz - message = 1
 * isHIGH = 0 | isLOW = 1 -> 900 Hz - message = 0
 * 
 * 
 * En r�sum�:
 * 
 * 1 symbole audio (0.1 ms) => �chantillonnage � 15 kHz ==> 1500 �ch. / symb.
 * 
 * Tous les 15 �chantillons, le groupe de mesure est filtr� et un indicateur concernant la pr�sence
 * de 900 ou 1100 Hz est lev� (ou non). On enregistre ces indicateurs et on recommence avec les 15 �chantillons
 * suivants
 * 
 * Une fois la p�riode symbole �coul�e, on a deux listes de 100 indicateurs. On regarde, pour 900 et 1100 Hz,
 * si la majorit� des indicateurs indiquent oui ou non la pr�sence de la fr�quence. On moyenne donc le tout pour
 * prendre une d�cision.Une fois la d�cision effectu�e, on se retrouve avec le contenu fr�quentiel de la p�riode
 * et on d�code le symbole.
 * 
 */

int decode100ms(void){
    //Initialisation des Listes de detLow/detHigh pour 1 p�riode symbole
    int detLOWlist[NBSAMPLES];
    int detHIGHlist[NBSAMPLES];
    
    int j = 0;
    
    while(!IFS0bits.T2IF && j<NBSAMPLES){   //Tant que la p�riode d'un symbole n'est pas finie
        sampleAndFilterOnePeriod();         // on �chantillonne et filtre
        detLOWlist[j] = detLOW;             // les r�sultats des filtres sont stock�s en attendant la
        detHIGHlist[j] = detHIGH;           // fin de la p�riode symbole
        j++;
    }
    IFS0bits.T2IF = 0; //Remise � z�ro du Flag
        
    // D�termination de la majorit� d�tect�e sur la p�riode symbole
    int isLOW = majorityInList(detLOWlist,NBSAMPLES);
    int isHIGH = majorityInList(detHIGHlist,NBSAMPLES);
    
    //Decodage    
    int msg = fskDetector(isLOW, isHIGH);
    
    return msg;    
}

int majorityInList(int* bitsList, int sizeOfList){
    /* 
     * Cette fonction d�termine si une majorit� de 1 ou 0 est
     * pr�sente au sein de la liste bitsList
     */
    int index = 0;
    int ones = 0;
    int zeros = 0;
    for (index=0;index<sizeOfList;index++){
        if (bitsList[index]){
            ones++;
        }else{
            zeros++;
        }
    }
    if (ones >= zeros){
        return 1;
    }else{
        return 0;
    }
}

void sampleAndFilterOnePeriod(void){
    //Initialisations de la liste d'�chantillons
    int measures[PERSAMPLES];
    
    int j=0;
    
    while(!IFS0bits.T2IF && j<PERSAMPLES){      //Tant qu'une p�riode de 15 �chantillons n'a pas
        if (IFS0bits.T3IF){                     //�t� acquise, on continue d'�chantillonner.
            IFS0bits.T3IF = 0;
            while (!adcConversionFinished());   //On attend la fin de la conversion
            measures[j] = adcRead();            //On place la mesure dans la liste
            j++;
        }
    }
    //On filtre alors le groupe de mesure, ce qui modifiera les variables globales detLOW et detHIGH
    runFilter(measures);
}

/*IIR Filter second order */
void IIRFilter(long Signal, long NumCoeff[3], long DenCoeff[4][3], long gain[4], long *Reg){
    /*Shift register + Add signal to input Signal*/
    Reg[2]=Reg[1];
    Reg[1]=Reg[0];
    Reg[0]=Signal;
    
    /*First stage*/
    Reg[REG_SIZE+2]= Reg[REG_SIZE+1];
    Reg[REG_SIZE+1]= Reg[REG_SIZE+0];
    Reg[REG_SIZE+0]= ((NumCoeff[0]*Reg[0] + NumCoeff[1]*Reg[1] + NumCoeff[2]*Reg[2] - DenCoeff[0][1]*Reg[REG_SIZE+1] - DenCoeff[0][2]*Reg[REG_SIZE+2]))/100000 ;
    
    /*Second stage*/
    Reg[2*REG_SIZE+2]= Reg[2*REG_SIZE+1];
    Reg[2*REG_SIZE+1]= Reg[2*REG_SIZE+0];
    Reg[2*REG_SIZE+0]= (((NumCoeff[0]*Reg[REG_SIZE+0] + NumCoeff[1]*Reg[REG_SIZE+1] + NumCoeff[2]*Reg[REG_SIZE+2])*gain[0])/100000 - DenCoeff[1][1]*Reg[2*REG_SIZE+1] - DenCoeff[1][2]*Reg[2*REG_SIZE+2])/100000 ;
    
    /*Third stage*/
    Reg[3*REG_SIZE+2]= Reg[3*REG_SIZE+1];
    Reg[3*REG_SIZE+1]= Reg[3*REG_SIZE+0];
    Reg[3*REG_SIZE+0]= (((NumCoeff[0]*Reg[2*REG_SIZE+0] + NumCoeff[1]*Reg[2*REG_SIZE+1] + NumCoeff[2]*Reg[2*REG_SIZE+2])*gain[1])/100000 - DenCoeff[2][1]*Reg[3*REG_SIZE+1] - DenCoeff[2][2]*Reg[3*REG_SIZE+2])/100000 ;
    
    /*Fourth stage*/
    Reg[4*REG_SIZE+2]= Reg[4*REG_SIZE+1];
    Reg[4*REG_SIZE+1]= Reg[4*REG_SIZE+0];
    Reg[4*REG_SIZE+0]= (((NumCoeff[0]*Reg[3*REG_SIZE+0] + NumCoeff[1]*Reg[3*REG_SIZE+1] + NumCoeff[2]*Reg[3*REG_SIZE+2])*gain[2])/100000 - DenCoeff[3][1]*Reg[4*REG_SIZE+1] - DenCoeff[3][2]*Reg[4*REG_SIZE+2])/100000 ;

}

/*Return the maximum value of a list*/
long maxValue(long *FilteredSignal,int SizeSignal){
 int i;
 long max=FilteredSignal[0];
 for(i=0;i<SizeSignal;i++){
     long evaluatedValue=FilteredSignal[i];
     if(evaluatedValue<0){
         evaluatedValue=-evaluatedValue;
     }
     if(max<evaluatedValue){
         max=evaluatedValue;
         
     }
 }
 return max;
 }

/*Shift value in a list and after add the new value in first position*/
void addToFilteredSignal(long *FilteredSignal, long input,int SizeSignal){
    int k;
    for(k=SizeSignal-1; k>0; k--)FilteredSignal[k] = FilteredSignal[k-1];
    FilteredSignal[0]=input;
}

/*Section1 IIR filter 900Hz*/
long  filter900(long Signal,long *Reg900){
    long NumCoeff[3] = {100000, 0,-100000};
    long DenCoeff[4][3] = {{100000,-184923,   99430},{100000,-185935,  99449},{100000,-184491,   98644},{100000,-184919,   98663}};
    long Gain[4]={731, 731, 728, 728} ;
    //long Gain[4]={1, 1, 1, 1};
    IIRFilter(Signal, NumCoeff, DenCoeff, Gain,  Reg900 );
    long y=((Reg900[4*REG_SIZE+0])*Gain[3])/100000;
    /*printf("Valeur de y: %f\n",y);*/
    return y;
}
/* Section2 IIR filter 1100Hz*/
long  filter1100(long Signal,long *Reg1100){
    long NumCoeff[3] = {100000, 0,-100000};
    long DenCoeff[4][3] = {{100000,-177777,  99304},{100000,-179266,  99327 },{100000,-177360 ,   98345},{100000,-177985,   98368}};
    long Gain[4]={893, 893, 889, 889} ;
    //long Gain[4]={1, 1, 1, 1};
    IIRFilter(Signal, NumCoeff, DenCoeff, Gain,  Reg1100 );
    long y=((Reg1100[4*REG_SIZE+0])*Gain[3])/100000;
    /*printf("Valeur de y: %f\n",y);*/
    return y;
}

long meanValue(long *list,int sizeList){
    int i;
    long meanValue=0;
    for (i=0;i<sizeList;i++){
        meanValue = meanValue+list[i];
    }
    meanValue = meanValue/sizeList;
    
    return meanValue;
}

void runFilter(long* listSignal){
    int i;
    for(i=0;i<15;i++){
        Reg900[i]=0;
        Reg1100[i]=0;
    }
    int j;
    for(j=0;j<SIZESIGNAL;j++){
        FilteredSignal900[j]=0;   //Define all list's values at 0
        FilteredSignal1100[j]=0;  //Define all list's values at 0
    }
    int k;
    for(k=0; k<PERSAMPLES; k++){
        long Signal = listSignal[k];
        long input900= filter900(Signal, Reg900); //Result of the 900Hz filter
        long input1100= filter1100(Signal, Reg1100); //Result of the 1100Hz filter
        addToFilteredSignal(FilteredSignal900, input900, SIZESIGNAL); //Add the 900Hz filtered value to the filtered Signal list
        addToFilteredSignal(FilteredSignal1100, input1100, SIZESIGNAL); //Add the 1100Hz filtered value to the filtered Signal list
    }
    long maxValue900=maxValue(FilteredSignal900,SIZESIGNAL); //Maximum value of the Filtered Signal 900 on one Periode
    if(THRESHOLDVALUE<maxValue900){ //compare the MaxValue900 with the THRESHOLDVALUE
        detLOW = 1;
    }
    long maxValue1100=maxValue(FilteredSignal1100,SIZESIGNAL); //Maximum value of the Filtered Signal 1100 on one Periode
    if(THRESHOLDVALUE<maxValue1100){ //compare the MaxValue1100 with the THRESHOLDVALUE
        detHIGH = 1;
    }
}