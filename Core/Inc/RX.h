/*
 * RX.h
 *
 *  Created on: Nov 17, 2020
 *      Author: jeffr
 */

#ifndef SRC_RX_H_
#define SRC_RX_H_

#include <stdint.h>
#include <stdlib.h>
#include "main.h"


typedef struct RX_CONTROLLER
{
	uint32_t throttle;				// Throttle data (TX Channel 3)
	uint32_t pitch;					// X-axis data (TX Channel 2)
	uint32_t roll;					// Y-axis data (TX Channel 1)
	uint32_t yaw; 					// Z-axis data (TX Channel 4)
	uint32_t switchA;				// Switch A State (TX Channel 5)
	uint32_t switchB;				// Switch B State (TX Channel 6)
	TIM_HandleTypeDef* timerSticks;
	TIM_HandleTypeDef* timerSwitches;
	DMA_HandleTypeDef* DMA;
} RX_CONTROLLER;

RX_CONTROLLER* RX_INIT(TIM_HandleTypeDef* timerSticks, TIM_HandleTypeDef* timerSwitches);
void RX_UPDATE(RX_CONTROLLER* RX_CONTROLLER);

#endif /* SRC_RX_H_ */
