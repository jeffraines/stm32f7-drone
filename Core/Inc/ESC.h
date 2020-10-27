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
} ESC_CONTROLLER;


/* Function Summary: Initiate the Electronic Speed Controller (esc) for a particular timer and channel.
 * Parameter: *timer - Pointer to predefined timer, channel - Channel number for the timer
// Recommended parameter inputs: *timer = &htim1, &htim2, &htim3, &htim4; channel = 1, ... , 6
 *
 */

ESC_CONTROLLER* INIT_ESC_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdma1,
															  DMA_HandleTypeDef* hdma2,
															  DMA_HandleTypeDef* hdma3,
															  DMA_HandleTypeDef* hdma4);


#endif /* INC_ESC_H_ */
