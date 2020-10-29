/*
 * ADC.c
 *
 *  Created on: Oct 28, 2020
 *      Author: Jeff Raines
 */

#include "main.h"
#include "ADC.h"

/* Function Summary: The input from the ADC handler will be piped directly to the inputVar via DMA.
 * Parameters: hadc - ADC handler for particular ADC pin. inputVar - Variable you want ADC values stored into.
 */

void ADC_INIT(ADC_HandleTypeDef* hadc, uint32_t* inputVar)
{
	HAL_ADC_Start_DMA(hadc, inputVar, sizeof(*inputVar));
	HAL_ADC_Start(hadc);
}

