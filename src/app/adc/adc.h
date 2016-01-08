#ifndef _ADC_H
#define _ADC_H

#include "pub.h"
#include "raw_api.h"

void ADC_InitConfiguration(void);
uint16_t ADC_Filter(uint8_t ch);

#endif
