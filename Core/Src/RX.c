/*
 * RX.c
 *
 *  Created on: Nov 17, 2020
 *      Author: Jeff Raines
 */

#include "RX.h"

RX_CONTROLLER* RX_INIT(TIM_HandleTypeDef* timerSticks, TIM_HandleTypeDef* timerSwitches, DMA_HandleTypeDef* DMA)
{
	RX_CONTROLLER* RX_CONTROLLER = malloc(sizeof(RX_CONTROLLER));
	RX_CONTROLLER->throttle = 0;
	RX_CONTROLLER->pitch = 0;
	RX_CONTROLLER->roll = 0;
	RX_CONTROLLER->yaw = 0;
	RX_CONTROLLER->switchA = 0;
	RX_CONTROLLER->switchB = 0;
	RX_CONTROLLER->timerSticks = timerSticks;
	RX_CONTROLLER->timerSwitches = timerSwitches;
	RX_CONTROLLER->DMA = DMA;
	return RX_CONTROLLER;
}

void RX_UPDATE(RX_CONTROLLER* RX_CONTROLLER)
{
	HAL_TIM_IC_Start_DMA(RX_CONTROLLER->timerSticks, TIM_CHANNEL_1, &RX_CONTROLLER->throttle, 4); // This will increment channel and data memory
	while(RX_CONTROLLER->DMA->Instance->CR & 0x1);
	HAL_TIM_IC_Start_DMA(RX_CONTROLLER->timerSticks, TIM_CHANNEL_1, &RX_CONTROLLER->switchA, 1);
	HAL_TIM_IC_Start_DMA(RX_CONTROLLER->timerSticks, TIM_CHANNEL_4, &RX_CONTROLLER->switchB, 1);
	while(RX_CONTROLLER->DMA->Instance->CR & 0x1);
}
