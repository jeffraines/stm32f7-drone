/*
 * ESC.h
 *
 *  Created on: Oct 24, 2020
 *      Author: Jeff Raines
 */

#ifndef INC_ESC_H_
#define INC_ESC_H_

typedef struct ESC
{
	uint32_t Throttle;
	uint32_t Channel;
	uint32_t Number;
	TIM_HandleTypeDef* Timer;
	DMA_HandleTypeDef* Dma;
} ESC_CONTROLLER;


/* Function Summary: Initiate the Electronic Speed Controller (ESC) for a particular timer and DMA streams.
 * Parameters: *timer - Pointer to predefined timer, *hdmaArray - Array containing set of DMA streams
 * Return: Pointer to the beginning of an array populated with ESC structs.
 */

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdmaArray[]);

/* Function Summary: Once the throttle has a new value loaded in this is called to start the output of that throttle value.
 * Parameters: ESC - Pointer to the single ESC_CONTROLLER that needs throttle to be updated.
 */

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC);

#endif /* INC_ESC_H_ */
