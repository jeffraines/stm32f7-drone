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

//#define DSHOT150
#define DSHOT300
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

#define DSHOT_MIN_THROTTLE	0
#define DSHOT_MAX_THROTTLE 	2047

#if defined(DSHOT150) || defined(DSHOT300) || defined(DSHOT600) || defined(DSHOT1200)

#define __DSHOT_CONSUME_BIT(__DSHOT_BYTE__, __BIT__) (__DSHOT_BYTE__ = (((__BIT__ & 0b1) == 0b1) ? DSHOT_HIGH_BIT : DSHOT_LOW_BIT))

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

uint16_t makeDshotPacketBytes(uint32_t value)
{
	uint16_t packet = (value << 1) | 1;
	int csum = 0;
	int csumData = value;
	for (int i = 0; i < 3; i++)
	{
		csum ^= csumData; // xor data by nibbles
		csumData >>= 4;
	}
	csum &= 0xf;
	packet = (packet << 4) | csum;
	return packet;
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle)
{
	// Throttle cannot exceed 11 bits, so max value is 2047
	if (throttle > DSHOT_MAX_THROTTLE) throttle = DSHOT_MAX_THROTTLE;
	else if (throttle < DSHOT_MIN_THROTTLE) throttle = DSHOT_MIN_THROTTLE;
 	uint16_t dshotBytes = makeDshotPacketBytes(throttle);
	// 17th bit is to set CCR to 0 to keep it low between packets
	uint32_t dshotPacket[17] = {0};
	dshotPacket[16] = 0;
	// Populate checksum bits
	for (int i = 15; i >= 0; i--)
	{
		__DSHOT_CONSUME_BIT(dshotPacket[i], dshotBytes);
		dshotBytes >>= 1;
	}
	// Setup the DMA stream to send the dshotPacket bytes to the CCR
	// Clear transfer and half transfer complete flags
	__HAL_DMA_CLEAR_FLAG(ESC->DMA, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4 |
									DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7));
	ESC->DMA->Instance->NDTR = 17;
	ESC->DMA->Instance->M0AR = (uint32_t) dshotPacket;
	ESC->DMA->Instance->PAR = (uint32_t) ESC->CCR;
	__HAL_DMA_ENABLE(ESC->DMA);
	while(ESC->DMA->Instance->CR & 0x1);
}

void ESC_CMD_SEND(ESC_CONTROLLER* ESC, uint32_t cmd)
{
	// 17th bit is to set CCR to 0 to keep it low between packets
	uint32_t dshotPacket[17] = {0};
	uint16_t dshotCMD = 0xffff;
	dshotCMD &= cmd;
	// Populate checksum bits
	for (int i = 15; i >= 0; i--)
	{
		__DSHOT_CONSUME_BIT(dshotPacket[i], dshotCMD);
		dshotCMD = dshotCMD >> 1;
	}
	// Setup the DMA stream to send the dshotPacket bytes to the CCR
	// Clear transfer and half transfer complete flags
	__HAL_DMA_CLEAR_FLAG(ESC->DMA, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4 |
									DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7));
	ESC->DMA->Instance->NDTR = 17;
	ESC->DMA->Instance->M0AR = (uint32_t) dshotPacket;
	ESC->DMA->Instance->PAR = (uint32_t) ESC->CCR;
	__HAL_DMA_ENABLE(ESC->DMA);
	while(ESC->DMA->Instance->CR & 0x1);
}

void ESC_SETTING(ESC_CONTROLLER* ESC, uint32_t setting)
{
	for (int i = 0; i < 10; i++)
	{
		ESC_CMD_SEND(ESC, setting);
	}
	for (int i = 0; i < 10; i++)
	{
		ESC_CMD_SEND(ESC, DSHOT_CMD_SAVE_SETTINGS);
	}
}

void ESC_STOP(ESC_CONTROLLER* ESC)
{
	for (int i = 0; i < 4; i++)
	{
		ESC_CMD_SEND(&ESC[i], DSHOT_CMD_MOTOR_STOP);
	}
}
#endif

#if defined(MULTISHOT) || defined(ONESHOT)

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* dma)
{
	timer->Instance->ARR = TIMER_ARR - 1;
	ESC_CONTROLLER* ESC_CONTROLLER = malloc(sizeof(ESC_CONTROLLER) * ESC_COUNT);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		ESC_CONTROLLER[i].Throttle = 0;
		ESC_CONTROLLER[i].Channel = 4*i;
		ESC_CONTROLLER[i].Number = i;
		ESC_CONTROLLER[i].Timer = timer;
		ESC_CONTROLLER[i].DMA = dma;
		ESC_CONTROLLER[i].CCR = (uint32_t) &(timer->Instance->CCR1) + (4*i);
		*((uint32_t *) ESC_CONTROLLER[i].CCR) = 0;
		HAL_TIM_PWM_Start(timer, ESC_CONTROLLER[i].Channel);
	}
	return ESC_CONTROLLER;
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle)
{
	__HAL_DMA_CLEAR_FLAG(ESC->DMA, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4));
	ESC->DMA->Instance->NDTR = 1;
	ESC->DMA->Instance->M0AR = (uint32_t) &throttle;
	ESC->DMA->Instance->PAR = ESC->CCR;
	__HAL_DMA_ENABLE(ESC->DMA);
	while(ESC->DMA->Instance->CR & 0x1);
 }
#endif











