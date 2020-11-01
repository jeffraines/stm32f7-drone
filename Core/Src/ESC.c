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

#include <stdint.h>
#include <stdlib.h>
#include "main.h"
#include "ESC.h"

#define DSHOT_THROTTLE_MASK 	0b1111111111100000	// DSHOT 11 bits for throttle
#define DSHOT_TELEMETRY_MASK 	0b0000000000010000 	// DSHOT 1 bit for telemetry
#define DSHOT_CHECKSUM_MASK		0b0000000000001111	// DSHOT 4 bits for checksum

#define DSHOT150
//#define DSHOT300
//#define DSHOT600
//#define DSHOT1200
//#define MULTISHOT

#ifdef DSHOT150
#define ARR				720 // Auto Reload Register
#define DSHOT_LOW_BIT	270 // Low logic PWM value for DSHOT150
#define DSHOT_HIGH_BIT 	540 // High logic PWM value for DSHOT150
#endif

#ifdef DSHOT300
#define ARR				360 // Auto Reload Register
#define DSHOT_LOW_BIT	135 // Low logic PWM value for DSHOT300
#define DSHOT_HIGH_BIT 	270 // High logic PWM value for DSHOT300
#endif

#ifdef DSHOT300
#define ARR				180 // Auto Reload Register
#define DSHOT_LOW_BIT	68 // Low logic PWM value for DSHOT600
#define DSHOT_HIGH_BIT 	135 // High logic PWM value for DSHOT600
#endif

#ifdef DSHOT300
#define ARR				90 // Auto Reload Register
#define DSHOT_LOW_BIT	34 // Low logic PWM value for DSHOT1200
#define DSHOT_HIGH_BIT 	68 // High logic PWM value for DSHOT1200
#endif

#if defined(DSHOT150) || defined(DSHOT300) || defined(DSHOT600) || defined(DSHOT1200)

#define __DSHOT_SEND_BIT(__ESC__, __BIT__) ((__ESC__)->Throttle = (((__BIT__ & 0b1) == 0b1) ? DSHOT_HIGH_BIT : DSHOT_LOW_BIT))

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdmaArray[])
{
	ESC_CONTROLLER* ESC_CONTROLLER = malloc(sizeof(ESC_CONTROLLER) * ESC_COUNT);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		ESC_CONTROLLER[i].Throttle = 0;
		ESC_CONTROLLER[i].Channel = 4*i;
		ESC_CONTROLLER[i].Number = i;
		ESC_CONTROLLER[i].Timer = timer;
		ESC_CONTROLLER[i].Dma = hdmaArray[i];
		HAL_DMA_Start(hdmaArray[i], (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
		HAL_TIM_PWM_Start(timer, ESC_CONTROLLER[i].Channel);
	}
	return ESC_CONTROLLER;
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle)
{
	if (throttle > 2048) throttle = 2047;											// Throttle cannot exceed 11 bits, so max value is 2047
	uint8_t telemtry =0b0;															// Updating only throttle value, so telemtry is 0
	uint8_t checksum = 0b0000;														// Updating only throttle value, so checksum is 0
	for (int checksumBits = 0; checksumBits < 4; checksumBits++)
		{
			__DSHOT_SEND_BIT(ESC, checksum);
			checksum = checksum >> 1;
			__HAL_DMA_CLEAR_FLAG(ESC->Dma, DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 |
										   DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 |
										   DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 |
										   DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7);	// Clear transfer and half transfer complete flags
			__HAL_DMA_SET_COUNTER(ESC->Dma, sizeof(ESC->Throttle));
			__HAL_DMA_ENABLE(ESC->Dma);
			HAL_TIM_PWM_Start(ESC->Timer, ESC->Channel);
		}
	for (int telemetryBit = 0; telemetryBit < 1; telemetryBit++)
		{
			__DSHOT_SEND_BIT(ESC, telemtry);
			__HAL_DMA_CLEAR_FLAG(ESC->Dma, DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 |
										   DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 |
										   DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 |
										   DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7);	// Clear transfer and half transfer complete flags
			__HAL_DMA_SET_COUNTER(ESC->Dma, sizeof(ESC->Throttle));
			__HAL_DMA_ENABLE(ESC->Dma);
			HAL_TIM_PWM_Start(ESC->Timer, ESC->Channel);
		}
	for (int throttleBits = 0; throttleBits < 11; throttleBits++)
	{
 		__DSHOT_SEND_BIT(ESC, throttle);
		throttle = throttle >> 1;
		__HAL_DMA_CLEAR_FLAG(ESC->Dma, DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 |
									   DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 |
									   DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 |
									   DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7);	// Clear transfer and half transfer complete flags
		__HAL_DMA_SET_COUNTER(ESC->Dma, sizeof(ESC->Throttle));
		__HAL_DMA_ENABLE(ESC->Dma);
		HAL_TIM_PWM_Start(ESC->Timer, ESC->Channel);
	}
}

void DSHOT_CMD_SEND(void)
{

}

#endif

#ifdef MULTISHOT

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdmaArray[])
{
	ESC_CONTROLLER* ESC_CONTROLLER = malloc(sizeof(ESC_CONTROLLER) * ESC_COUNT);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		ESC_CONTROLLER[i].Throttle = 0;
		ESC_CONTROLLER[i].Channel = 4*i;
		ESC_CONTROLLER[i].Number = i;
		ESC_CONTROLLER[i].Timer = timer;
		ESC_CONTROLLER[i].Dma = hdmaArray[i];
		HAL_DMA_Start(hdmaArray[i], (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
		HAL_TIM_PWM_Start(timer, ESC_CONTROLLER[i].Channel);
	}
	return ESC_CONTROLLER;
}

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC)
{
	// May want to handle error checking for DMA_FLAG_(TEIFx, DMEIFx, FEIFx) flags
	__HAL_DMA_CLEAR_FLAG(ESC->Dma, DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 |
						 	   DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 |
							   DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 |
							   DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7);	// Clear transfer and half transfer complete flags
	__HAL_DMA_SET_COUNTER(ESC->Dma, sizeof(ESC->Throttle));
	__HAL_DMA_ENABLE(ESC->Dma);
	HAL_TIM_PWM_Start(ESC->Timer, ESC->Channel);
 }
#endif











