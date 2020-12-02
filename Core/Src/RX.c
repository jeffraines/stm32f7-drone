/*
 * RX.c
 *
 *  Created on: Nov 17, 2020
 *      Author: Jeff Raines
 */

#include "RX.h"

RX_CONTROLLER* RX_INIT(TIM_HandleTypeDef* timerSticks, TIM_HandleTypeDef* timerSwitches)
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
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSticks, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSticks, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSticks, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSticks, TIM_CHANNEL_4);
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSwitches, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(RX_CONTROLLER->timerSwitches, TIM_CHANNEL_4);
	return RX_CONTROLLER;
}

// TO DO: Debug this
// TO DO: Convert the values before putting them RX_CONTROLLER. Probably best to load into local array then copy into RX_CONTROLLER
void RX_UPDATE(RX_CONTROLLER* RX_CONTROLLER)
{
	uint32_t curThrottle = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSticks, TIM_CHANNEL_1);
	curThrottle = (curThrottle - 998) * 2.045;
	RX_CONTROLLER->throttle = curThrottle;
	RX_CONTROLLER->pitch = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSticks, TIM_CHANNEL_2);
	RX_CONTROLLER->roll = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSticks, TIM_CHANNEL_3);
	RX_CONTROLLER->yaw = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSticks, TIM_CHANNEL_4);
	RX_CONTROLLER->switchA = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSwitches, TIM_CHANNEL_1);
	RX_CONTROLLER->switchB = HAL_TIM_ReadCapturedValue(RX_CONTROLLER->timerSwitches, TIM_CHANNEL_4);
}
