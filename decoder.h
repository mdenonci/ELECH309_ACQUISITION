#ifndef DECODER_H
#define	DECODER_H

#include <xc.h> // include processor files - each processor file is guarded.  

int decode100ms(void);

int majorityInList(int* bitsList, int sizeOfList);

void sampleAndFilterOnePeriod(void);

void IIRFilter(long Signal, long NumCoeff[3], long DenCoeff[4][3], long gain[4], long *Reg);

long maxValue(long *FilteredSignal,int SizeSignal);

void addToFilteredSignal(long *FilteredSignal, long input,int SizeSignal);

long  filter900(long Signal,long *Reg900);

long  filter1100(long Signal,long *Reg1100);

long meanValue(long *list,int sizeList);

void runFilter(long* Signal);

#endif	/* XC_HEADER_TEMPLATE_H */

