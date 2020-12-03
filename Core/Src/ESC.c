/*
 * ESC.c
 *
 *  Created on: Oct 24, 2020
 *      Author: Jeff Raines
 */

/** BLHeili DSHOT Protocol Setup
Packet Info - 16 bits - | 15-5 = throttle | 4 = telemetry | 3-0 checksum |

| 1| 0| 0| 0| 0| 0| 1| 0| 1| 1| 0| 0| 0| 1| 1| 0|
|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
|    Throttle Value 0 - 2048     | T| checksum  |

DSHOT Decoding Timing
- DSHOT150 = 150kB/S (150kHz), 6.67uS bit length, 106.67uS packet period (16 bits)
 	* 1 Bit Pulse Width Logic - 0 = 2.5uS H | 1 = 5uS H
 	* Prescaler = 0, Counter Period (ARR) = 719

- DSHOT300 = 300kB/S (300kHz), 3.33uS bit length, 53.330uS packet period (16 bits)
 	* 1 Bit Pulse Width Logic - 0 = 1.25uS H | 1 = 2.5uS H
 	* Prescaler = 0, Counter Period (ARR) = 359

- DSHOT600 = 600kB/S (600kHz), 1.67uS bit length, 26.670uS packet period (16 bits)
	* 1 Bit Pulse Width Logic - 0 = 625nS H | 1 = 1250nS H
	* Prescaler = 0, Counter Period (ARR) = 179

- DSHOT1200 = 1200kB/S (1.2MHz), 0.83uS bit length, 13.3uS packet period (16 bits)
	* 1 Bit Pulse Width Logic - 0 = 312.5nS H | 1 = 625nS H
	* Prescaler = 0, Counter Period (ARR) = 89

Prescaler = (Timer Freq (Hz) / (PWM Freq (Hz) * (Counter Period) + 1) - 1)

* 0 Logic = 37.3, 37.5, 37.4, 37.6 ~ 37.3% - 37.6% on time
* 1 Logic = 75.0, 75.1m, 74.9, 75.3 ~ 74.9% - 73.3% on time
*/

#include "ESC.h"

#define DSHOT150
//#define DSHOT300
//#define DSHOT600
//#define DSHOT1200
//#define MULTISHOT
//#define ONESHOT

#ifdef DSHOT150
#define TIMER_ARR		720 // Auto Reload Register
#define DSHOT_LOW_BIT	270 // Low logic PWM value for DSHOT150
#define DSHOT_HIGH_BIT 	540 // High logic PWM value for DSHOT150
#endif

#ifdef DSHOT300
#define TIMER_ARR		360 // Auto Reload Register
#define DSHOT_LOW_BIT	135 // Low logic PWM value for DSHOT300
#define DSHOT_HIGH_BIT 	270 // High logic PWM value for DSHOT300
#endif

#ifdef DSHOT600
#define TIMER_ARR		180 // Auto Reload Register
#define DSHOT_LOW_BIT	68 	// Low logic PWM value for DSHOT600
#define DSHOT_HIGH_BIT 	135 // High logic PWM value for DSHOT600
#endif

#ifdef DSHOT1200
#define TIMER_ARR		90 	// Auto Reload Register
#define DSHOT_LOW_BIT	34 	// Low logic PWM value for DSHOT1200
#define DSHOT_HIGH_BIT 	68 	// High logic PWM value for DSHOT1200
#endif

#ifdef MULTISHOT
#define TIMER_ARR 		1350 // Auto Reload Register
#endif

#ifdef ONESHOT
#define TIMER_ARR 		4500 // Auto Reload Register
#endif

#define DSHOT_MIN_THROTTLE	32
#define DSHOT_MAX_THROTTLE 	2047

#if defined(DSHOT150) || defined(DSHOT300) || defined(DSHOT600) || defined(DSHOT1200)

#define __DSHOT_CONSUME_BIT(__DSHOT_BYTE__, __BIT__) (__DSHOT_BYTE__ = (((__BIT__ & 0b1) == 0b1) ? DSHOT_HIGH_BIT : DSHOT_LOW_BIT))

void ESC_CPLT_CALLBACK(ESC_CONTROLLER* thisEsc)
{

}

void ESC_HALF_CALLBACK(ESC_CONTROLLER* thisEsc)
{

}

ESC_CONTROLLER* ESC_INIT(TIM_HandleTypeDef** dmaTickTimers, TIM_HandleTypeDef* pwmTimer, DMA_HandleTypeDef** dmaHandlers)
{
	dmaTickTimers[0]->Instance->ARR = TIMER_ARR - 1; 	// htim4 ARR, synchronize timer that control DMA requests
	dmaTickTimers[1]->Instance->ARR = TIMER_ARR - 1; 	// htim5 ARR, synchronize timer that control DMA requests
	pwmTimer->Instance->ARR = TIMER_ARR - 1;		 		// htim3 ARR, synchronize timer that control DMA requests
	// Enable DMA requests on CH1 and CH2
	dmaTickTimers[0]->Instance->DIER = TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE;
	dmaTickTimers[1]->Instance->DIER = TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	HAL_TIM_PWM_Start(dmaTickTimers[0], TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(dmaTickTimers[0], TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(dmaTickTimers[0], TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(dmaTickTimers[1], TIM_CHANNEL_2);
	int bytes = sizeof(ESC_CONTROLLER)*ESC_COUNT;
	ESC_CONTROLLER* escSet = malloc(bytes);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		escSet[i].Throttle = 0;
		for (int j = 0; j < DSHOT_PACKET_SIZE; j++) escSet[i].ThrottleDshot[j] = 0;
		escSet[i].Channel = 4*i;
		escSet[i].Number = i;
		escSet[i].Timer = pwmTimer;
		escSet[i].DMA = dmaHandlers[i];
 		escSet[i].CCR = &(pwmTimer->Instance->CCR1) + i;
		*escSet[i].CCR = 0;
		void (*cpltCallback) = &ESC_CPLT_CALLBACK;
		void (*halfCallback) = &ESC_HALF_CALLBACK;
		HAL_DMA_RegisterCallback(escSet[i].DMA, HAL_DMA_XFER_CPLT_CB_ID, cpltCallback);
		HAL_DMA_RegisterCallback(escSet[i].DMA, HAL_DMA_XFER_HALFCPLT_CB_ID, halfCallback);
	}
	for (int i = 0; i < ESC_COUNT; i++)
	{
		HAL_TIM_PWM_Start(pwmTimer, escSet[i].Channel);
		HAL_DMA_Start_IT(escSet[i].DMA, (uint32_t) &escSet[i].ThrottleDshot,
								(uint32_t) escSet[i].CCR, DSHOT_PACKET_SIZE);
	}
	return escSet;
}

uint16_t makeDshotPacketBytes(uint32_t value, uint8_t telemBit)
{
	uint16_t packet = (value << 1) | telemBit;
	int csum = 0;
	int csumData = packet;
	for (int i = 0; i < 3; i++)
	{
		csum ^= csumData; // xor data by nibbles
		csumData >>= 4;
	}
	csum &= 0xf;
	packet = (packet << 4) | csum;
	return packet;
}

void DSHOT_SEND_PACKET(ESC_CONTROLLER* ESC, uint32_t data, uint32_t telemBit, uint32_t motorNum)
{
	uint16_t dshotBytes = makeDshotPacketBytes(data, telemBit);
	// 17th bit is to set CCR to 0 to keep it low between packets
	uint32_t dshotPacket[DSHOT_PACKET_SIZE] = {0};
	// Populate checksum bits
	for (int i = 15; i >= 0; i--)
	{
		__DSHOT_CONSUME_BIT(dshotPacket[i], dshotBytes);
		dshotBytes >>= 1;
	}
	switch(motorNum) {
		case (FRONT_LEFT_MOTOR):
			memcpy(ESC[0].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (FRONT_RIGHT_MOTOR):
			memcpy(ESC[1].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (BACK_LEFT_MOTOR):
			memcpy(ESC[2].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (BACK_RIGHT_MOTOR):
			memcpy(ESC[3].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (LEFT_SIDE_MOTORS):
			memcpy(ESC[0].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[2].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (RIGHT_SIDE_MOTORS):
			memcpy(ESC[1].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[3].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (FRONT_SIDE_MOTORS):
			memcpy(ESC[0].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[1].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (BACK_SIDE_MOTORS):
			memcpy(ESC[2].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[3].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
		case (ALL_MOTORS):
			memcpy(ESC[0].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[1].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[2].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			memcpy(ESC[3].ThrottleDshot, dshotPacket, sizeof(dshotPacket));
			break;
	}
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle)
{
	// Throttle cannot exceed 11 bits, so max value is 2047
	if (throttle > DSHOT_MAX_THROTTLE) throttle = DSHOT_MAX_THROTTLE;
	else if (throttle < DSHOT_MIN_THROTTLE) throttle = DSHOT_MIN_THROTTLE;
	DSHOT_SEND_PACKET(ESC, throttle, 0);
}

void ESC_SEND_CMD(ESC_CONTROLLER* ESC, uint32_t cmd)
{
	// Need to set telemetry bit to 1 if either of these commands are sent
	if (cmd == 	DSHOT_CMD_SPIN_DIRECTION_NORMAL || DSHOT_CMD_SPIN_DIRECTION_REVERSED ||
				DSHOT_CMD_3D_MODE_ON || DSHOT_CMD_3D_MODE_OFF ||
				DSHOT_CMD_SPIN_DIRECTION_1 || DSHOT_CMD_SPIN_DIRECTION_2)
	{
		for (int i = 0; i < 10; i++) DSHOT_SEND_PACKET(ESC, cmd, 1);
	}
	else
	{
		DSHOT_SEND_PACKET(ESC, cmd, 0);
	}
	for (int i = 0; i < 10; i++) DSHOT_SEND_PACKET(ESC, DSHOT_CMD_SAVE_SETTINGS, 1);
}

#endif

#if defined(MULTISHOT) || defined(ONESHOT)

ESC_CONTROLLER* ESC_INIT(TIM_HandleTypeDef* dmaTimerTick, TIM_HandleTypeDef* pwmTimer, DMA_HandleTypeDef* dma)
{
	dmaTimerTick->Instance->ARR = TIMER_ARR - 1;
	pwmTimer->Instance->ARR = TIMER_ARR - 1;
	HAL_TIM_PWM_Start(dmaTimerTick, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(dmaTimerTick, TIM_CHANNEL_2);
	int bytes = sizeof(ESC_CONTROLLER) * ESC_COUNT;
	ESC_CONTROLLER* ESC_CONTROLLER = malloc(bytes);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		ESC_CONTROLLER[i].Throttle = 0;
		ESC_CONTROLLER[i].Channel = 4*i;
		ESC_CONTROLLER[i].Number = i;
		ESC_CONTROLLER[i].Timer = pwmTimer;
		ESC_CONTROLLER[i].DMA = &dma[i%2];
 		ESC_CONTROLLER[i].CCR = &(pwmTimer->Instance->CCR1) + i;
		*ESC_CONTROLLER[i].CCR = 0;
		HAL_TIM_PWM_Start(pwmTimer, ESC_CONTROLLER[i].Channel);
	}
	return ESC_CONTROLLER;
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle)
{
	__HAL_DMA_CLEAR_FLAG(ESC->DMA, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4 |
									DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7));
	ESC->DMA->Instance->NDTR = 1;
	ESC->DMA->Instance->M0AR = &throttle;
	ESC->DMA->Instance->PAR = (uint32_t) ESC->CCR;
	__HAL_DMA_ENABLE(ESC->DMA);
	while(ESC->DMA->Instance->CR & 0x1);
}

#endif











