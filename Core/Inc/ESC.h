/*
 * ESC.h
 *
 *  Created on: Oct 24, 2020
 *      Author: Jeff Raines
 */

#ifndef INC_ESC_H_
#define INC_ESC_H_

#include <stdint.h>
#include <stdlib.h>
#include "main.h"

/* Known ESC Commands */
#define DSHOT_CMD_MOTOR_STOP 							 	 0 	// |0000000000000000|
#define DSHOT_CMD_SPIN_DIRECTION_NORMAL 					20	// |0000000000010100|
#define DSHOT_CMD_SPIN_DIRECTION_REVERSED 				 	21	// |0000000000010101|
#define DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF 					30 	// |0000000000011110|
#define DSHOT_CMD_SILENT_MODE_ON_OFF 					 	31	// |0000000000011111|
#define DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE 		 	32	// |0000000000100000|
#define DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY	 	33	// |0000000000100001|
#define DSHOT_CMD_MAX 									 	47	// |0000000000101111|

/* Unknown ESC Commands */
#define DSHOT_CMD_3D_MODE_OFF							 	16 	// |0000000000010000|
#define DSHOT_CMD_3D_MODE_ON							 	17	// |0000000000010001|
#define DSHOT_CMD_SETTINGS_REQUEST						 	18	// |0000000000010010|
#define DSHOT_CMD_SAVE_SETTINGS							 	19	// |0000000000010011|

#define ONESHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = ((ADC_VALUE / 6.07) + 675))
#define MULTISHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = ((ADC_VALUE / 1.82) + 2250))
#define DSHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = (ADC_VALUE - 1600))

typedef struct ESC
{
	uint32_t Throttle;
	uint32_t Channel;
	uint32_t Number;
	TIM_HandleTypeDef* Timer;
	DMA_HandleTypeDef* DMA;
	volatile uint32_t* CCR;
} ESC_CONTROLLER;


/* Function Summary: Initiate the Electronic Speed Controller (ESC) for a particular timer and DMA streams.
 * Parameters: *timer - Pointer to predefined timer, *hdmaArray - Array containing set of DMA streams
 * Return: Pointer to the beginning of an array populated with ESC structs.
 */

ESC_CONTROLLER* ESC_INIT(TIM_HandleTypeDef* dmaTimerTick, TIM_HandleTypeDef* pwmTimer, DMA_HandleTypeDef* dma);

/* Function Summary: Once the throttle has a new value loaded in this is called to start the output of that throttle value.
 * Parameters: ESC - Pointer to the single ESC_CONTROLLER that needs throttle to be updated.
 */

void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC, uint32_t throttle);
void ESC_CMD_SEND(ESC_CONTROLLER* ESC, uint32_t cmd);
void ESC_SETTING(ESC_CONTROLLER* ESC, uint32_t setting);
void ESC_STOP(ESC_CONTROLLER* ESC);

#endif /* INC_ESC_H_ */
