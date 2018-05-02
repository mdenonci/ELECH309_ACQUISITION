
//#define DEBUG_FSK_DETECTOR

#define MESSAGE_LENGTH      10                          // nombre de bits de donnees de la trame

// On échantillonne à 15 kHz, on regroupe ensuite 15 échantillons pour créer 1 "échantillon audio" du signal à partir
// duquel on cherchera la fréquence - On aura donc 1000 échantillons audio par seconde
#define SAMPLING_FREQ       1000                         // frequence d'échantillonnage du signal audio, en Hz
#define BIT_FREQ            10                          // frequence des bits de la trame, en Hz
#define OSR                 (SAMPLING_FREQ/BIT_FREQ)    // OverSampling Ratio
#define FSK_MIN_SAMPLES_NB  (3*OSR/4)                   // nombre d'echantillons minimum pour considerer un bit valide

int fskDetector(int detLow, int detHigh);
