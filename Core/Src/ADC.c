/*
 * ADC.c
 *
 *  Created on: Oct 28, 2020
 *      Author: Jeff Raines
 */

#include "main.h"
#include "ADC.h"

/* Function Summary: Initiate and start ADC transferring data via DMA to input var
 * Param: * hadc - Pointer to predefined adc handler,
 * Param: * input - pointer to where ADC data should be stored
 * Return: VOID
 */
void ADC_INIT(ADC_HandleTypeDef* hadc, uint32_t* inputVar)
{
	HAL_ADC_Start_DMA(hadc, inputVar, sizeof(*inputVar));
	HAL_ADC_Start(hadc);
}

