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

Known ESC Commands
--------------------
DSHOT_CMD_MOTOR_STOP 							= 0  	|0000000000000000|
DSHOT_CMD_SPIN_DIRECTION_NORMAL 				= 20 	|0000000000010100|
DSHOT_CMD_SPIN_DIRECTION_REVERSED 				= 21	|0000000000010101|
DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF 				= 30 	|0000000000011110|
DSHOT_CMD_SILENT_MODE_ON_OFF 					= 31	|0000000000011111|
DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE 		= 32	|0000000000100000|
DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY	= 33	|0000000000100001|
DSHOT_CMD_MAX 									= 47	|0000000000101111|

Unknown ESC Commands
--------------------
DSHOT_CMD_3D_MODE_OFF							= 16 	|0000000000010000|
DSHOT_CMD_3D_MODE_ON							= 17	|0000000000010001|
DSHOT_CMD_SETTINGS_REQUEST						= 18	|0000000000010010|
DSHOT_CMD_SAVE_SETTINGS							= 19	|0000000000010011|
*/

//#define TIM_CHANNEL_1                      0x00000000U	0b000000
//#define TIM_CHANNEL_2                      0x00000004U	0b000100
//#define TIM_CHANNEL_3                      0x00000008U	0b001000
//#define TIM_CHANNEL_4                      0x0000000CU	0b001100
//#define TIM_CHANNEL_5                      0x00000010U	0b010000
//#define TIM_CHANNEL_6                      0x00000014U	0b010100
//#define TIM_CHANNEL_ALL                    0x0000003CU	0b111100

#include "ESC.h"

#define DSHOT_THROTTLE_MASK 	0b1111111111100000	// DSHOT 11 bits for throttle
#define DSHOT_TELEMETRY_MASK 	0b0000000000010000 	// DSHOT 1 bit for telemetry
#define DSHOT_CHECKSUM_MASK		0b0000000000001111	// DSHOT 4 bits for checksum

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

#if defined(DSHOT150) || defined(DSHOT300) || defined(DSHOT600) || defined(DSHOT1200)

#define __DSHOT_MAKE_BYTE(__DSHOT_BYTE__, __BIT__) (__DSHOT_BYTE__ = (((__BIT__ & 0b1) == 0b1) ? DSHOT_HIGH_BIT : DSHOT_LOW_BIT))

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

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle, uint8_t telemetry, uint8_t checksum)
{
	if(!telemetry) checksum = 0;
	// Throttle cannot exceed 11 bits, so max value is 2047
	if (throttle >= 2048) throttle = 2047;
	// 17th bit is to set CCR to 0 to keep it low between packets
	uint32_t dshotPacket[17] = {0};
	dshotPacket[16] = 0;
	// Populate checksum bits
	for (int checksumBits = 15; checksumBits >= 11; checksumBits--)
	{
		__DSHOT_MAKE_BYTE(dshotPacket[checksumBits], checksum);
		checksum = checksum >> 1;
	}
	// Populate telemetry bit
	//__DSHOT_MAKE_BYTE(dshotPacket[11], telemetry);
	// Populate throttle bits
	for (int throttleBits = 10; throttleBits >=0; throttleBits--)	{
		__DSHOT_MAKE_BYTE(dshotPacket[throttleBits], throttle);
		throttle = throttle >> 1;
	}
	// Setup the DMA stream to send the dshotPacket bytes to the CCR
	// Clear transfer and half transfer complete flags
	__HAL_DMA_CLEAR_FLAG(ESC->DMA, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4));
	ESC->DMA->Instance->NDTR = 17;
	ESC->DMA->Instance->M0AR = (uint32_t) &dshotPacket;
	ESC->DMA->Instance->PAR = ESC->CCR;
	__HAL_DMA_ENABLE(ESC->DMA);
	while(ESC->DMA->Instance->CR & 0x1);
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











