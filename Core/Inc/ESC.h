/*
 * ESC.h
 *
 *  Created on: Oct 24, 2020
 *      Author: Jeff Raines
 */

#ifndef INC_ESC_H_
#define INC_ESC_H_


// Add htim and hdma pointers, so that instead of having to pass those in to, they're just handled
typedef struct ESC
{
	uint32_t Throttle;
	uint32_t Channel;
	uint32_t Index;
} ESC_CONTROLLER;


/* Function Summary: Initiate the Electronic Speed Controller (esc) for a particular timer and channel.
 * Parameter: *timer - Pointer to predefined timer, channel - Channel number for the timer
// Recommended parameter inputs: *timer = &htim1, &htim2, &htim3, &htim4; channel = 1, ... , 6
 *
 */

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdma1, DMA_HandleTypeDef* hdma2, DMA_HandleTypeDef* hdma3, DMA_HandleTypeDef* hdma4);
void ESC_UPDATE_THROTTLE(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdma, ESC_CONTROLLER* ESC);

#endif /* INC_ESC_H_ */
