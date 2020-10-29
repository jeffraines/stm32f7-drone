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
- DSHOT300 = 300kB/S (300kHz), 3.33uS bit length, 53.330uS packet period (16 bits)
 	* 1 Bit Pulse Width Logic - 0 = 1.25uS H | 1 = 2.5uS H
- DSHOT600 = 600kB/S (600kHz), 1.67uS bit length, 26.670uS packet period (16 bits)
	* 1 Bit Pulse Width Logic - 0 = 625nS H | 1 = 1250nS H
- DSHOT1200 = 1200kB/S (1.2MHz), 0.83uS bit length, 13.3uS packet period (16 bits)
	* 1 Bit Pulse Width Logic - 0 = 312.5nS H | 1 = 625nS H

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

ESC_CONTROLLER* ESC_INIT_CONTROLLER(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdma1, DMA_HandleTypeDef* hdma2, DMA_HandleTypeDef* hdma3, DMA_HandleTypeDef* hdma4)
{
	ESC_CONTROLLER* ESC_CONTROLLER = malloc(sizeof(ESC_CONTROLLER) * ESC_COUNT);
	for (int i = 0; i < ESC_COUNT; i++)
	{
		ESC_CONTROLLER[i].Throttle = 1000*i;
		ESC_CONTROLLER[i].Channel = 4*i;
		ESC_CONTROLLER[i].Index = i;
		switch (i)
		{
			case 0:
				HAL_DMA_Start(hdma1, (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
				break;
			case 1:
				HAL_DMA_Start(hdma2, (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
				break;
			case 2:
				HAL_DMA_Start(hdma3, (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
				break;
			case 3:
				HAL_DMA_Start(hdma4, (uint32_t) &ESC_CONTROLLER[i].Throttle, (uint32_t) &timer->Instance->CCR1 + (4*i), sizeof(ESC_CONTROLLER[i].Throttle));
				break;
		}
		HAL_TIM_PWM_Start(timer, ESC_CONTROLLER[i].Channel);
	}
	return ESC_CONTROLLER;
}

void ESC_UPDATE_THROTTLE(TIM_HandleTypeDef* timer, DMA_HandleTypeDef* hdma, ESC_CONTROLLER* ESC)
{
	// May want to handle error checking for DMA_FLAG_(TEIFx, DMEIFx, FEIFx) flags
	__HAL_DMA_CLEAR_FLAG(hdma, DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 |
						 	   DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 |
							   DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 |
							   DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7);	// Clear transfer and half transfer complete flags
	__HAL_DMA_SET_COUNTER(hdma, sizeof(ESC[ESC->Channel].Throttle));
	__HAL_DMA_ENABLE(hdma);
	HAL_TIM_PWM_Start(timer, ESC->Channel);
 }












