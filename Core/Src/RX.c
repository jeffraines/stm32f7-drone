/*
 * RX.c
 *
 *  Created on: Nov 17, 2020
 *      Author: Jeff Raines
 */

#include "RX.h"

#define RX_EXPONENT 		2.045
#define RX_OFFSET 			998
#define RX_SWITCH_OFFSET	550


RX_CONTROLLER* RX_INIT(TIM_HandleTypeDef* timerSticks, TIM_HandleTypeDef* timerSwitches)
{
	RX_CONTROLLER* newRX = malloc(sizeof(RX_CONTROLLER));
	newRX->throttle = 0;
	newRX->pitch = 0;
	newRX->roll = 0;
	newRX->yaw = 0;
	newRX->switchA = 0;
	newRX->switchB = 0;
	newRX->timerSticks = timerSticks;
	newRX->timerSwitches = timerSwitches;
	HAL_TIM_IC_Start_IT(newRX->timerSticks, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(newRX->timerSticks, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(newRX->timerSticks, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(newRX->timerSticks, TIM_CHANNEL_4);
	HAL_TIM_IC_Start_IT(newRX->timerSwitches, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(newRX->timerSwitches, TIM_CHANNEL_4);
	return newRX;
}

void RX_UPDATE(RX_CONTROLLER* thisRX)
{
	// Calculate RX throttle value
	uint32_t curThrottle = HAL_TIM_ReadCapturedValue(thisRX->timerSticks, TIM_CHANNEL_1);
	if (curThrottle > RX_OFFSET - 1) curThrottle = (curThrottle - RX_OFFSET) * RX_EXPONENT;
	thisRX->throttle = curThrottle;
	// Calculate RX pitch value
	int32_t curPitch = HAL_TIM_ReadCapturedValue(thisRX->timerSticks, TIM_CHANNEL_2);
	if (curPitch > RX_OFFSET - 1) curPitch = (curPitch - RX_OFFSET) * RX_EXPONENT;
	thisRX->pitch = curPitch;
	// Calculate RX roll value
	uint32_t curRoll = HAL_TIM_ReadCapturedValue(thisRX->timerSticks, TIM_CHANNEL_3);
	if (curRoll > RX_OFFSET - 1) curRoll = (curRoll - RX_OFFSET) * RX_EXPONENT;
	thisRX->roll = curRoll;
	// Calculate RX yaw value
	uint32_t curYaw = HAL_TIM_ReadCapturedValue(thisRX->timerSticks, TIM_CHANNEL_4);
	if (curYaw > RX_OFFSET - 1) curYaw = (curYaw - RX_OFFSET) * RX_EXPONENT;
	thisRX->yaw = curYaw;
	// Calculate Switch A state
	uint32_t curSwitchA = HAL_TIM_ReadCapturedValue(thisRX->timerSwitches, TIM_CHANNEL_1);
	if (curSwitchA < RX_SWITCH_OFFSET) thisRX->switchA = 0;
	else thisRX->switchA = 1;
	// Calculate Switch B state
	uint32_t curSwitchB = HAL_TIM_ReadCapturedValue(thisRX->timerSwitches, TIM_CHANNEL_4);
	if (curSwitchB < RX_SWITCH_OFFSET) thisRX->switchB = 0;
	else thisRX->switchB = 1;
}

void RX_DISCONNECTED(RX_CONTROLLER* thisRX)
{
	thisRX->throttle = 0;
	thisRX->pitch = 0;
	thisRX->roll = 0;
	thisRX->yaw = 0;
	thisRX->switchA = 0;
	thisRX->switchB = 0;
}
