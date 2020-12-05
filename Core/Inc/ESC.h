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
#include <string.h>
#include "main.h"
#include "RX.h"

#define DSHOT_PACKET_SIZE 	32
#define ESC_COUNT 4

typedef enum {
    DSHOT_CMD_MOTOR_STOP = 0,
    DSHOT_CMD_BEACON1,
    DSHOT_CMD_BEACON2,
    DSHOT_CMD_BEACON3,
    DSHOT_CMD_BEACON4,
    DSHOT_CMD_BEACON5,
    DSHOT_CMD_ESC_INFO, // V2 includes settings
    DSHOT_CMD_SPIN_DIRECTION_1,	// Counter-Clockwise
    DSHOT_CMD_SPIN_DIRECTION_2, // Clockwise
    DSHOT_CMD_3D_MODE_OFF,
    DSHOT_CMD_3D_MODE_ON,
    DSHOT_CMD_SETTINGS_REQUEST, // Currently not implemented
    DSHOT_CMD_SAVE_SETTINGS,
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    DSHOT_CMD_LED0_ON, // BLHeli32 only
    DSHOT_CMD_LED1_ON, // BLHeli32 only
    DSHOT_CMD_LED2_ON, // BLHeli32 only
    DSHOT_CMD_LED3_ON, // BLHeli32 only
    DSHOT_CMD_LED0_OFF, // BLHeli32 only
    DSHOT_CMD_LED1_OFF, // BLHeli32 only
    DSHOT_CMD_LED2_OFF, // BLHeli32 only
    DSHOT_CMD_LED3_OFF, // BLHeli32 only
    DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF = 30, // KISS audio Stream mode on/Off
    DSHOT_CMD_SILENT_MODE_ON_OFF = 31, // KISS silent Mode on/Off
	DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE = 32,
	DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY = 33,
    DSHOT_CMD_MAX = 47
} dshotCommands_e;

typedef enum {
	FRONT_LEFT_MOTOR = 0,
	FRONT_RIGHT_MOTOR,
	BACK_LEFT_MOTOR,
	BACK_RIGHT_MOTOR,
	LEFT_SIDE_MOTORS,
	RIGHT_SIDE_MOTORS,
	FRONT_SIDE_MOTORS,
	BACK_SIDE_MOTORS,
	ALL_MOTORS
} motors;

typedef struct ESC
{
	uint32_t Throttle[ESC_COUNT];
	uint32_t ThrottleDshot[ESC_COUNT][DSHOT_PACKET_SIZE];
	uint32_t Channel[ESC_COUNT];
	TIM_HandleTypeDef* Timer[ESC_COUNT];
	DMA_HandleTypeDef* DMA[ESC_COUNT];
	volatile uint32_t* CCR[ESC_COUNT];
} ESC_CONTROLLER;

typedef struct
{
	uint32_t FrontLeft;
	uint32_t FrontRight;
	uint32_t BackLeft;
	uint32_t BackRight;
} MOTOR_THROTTLES;

/* Function Summary: Initiate the Electronic Speed Controller (ESC) for a particular timer and DMA streams.
 * Parameters: *timer - Pointer to predefined timer, *hdmaArray - Array containing set of DMA streams
 * Return: Pointer to the beginning of an array populated with ESC structs.
 */

ESC_CONTROLLER* ESC_INIT(TIM_HandleTypeDef** dmaTickTimers, TIM_HandleTypeDef* pwmTimer, DMA_HandleTypeDef** dma);

/* Function Summary: Once the throttle has a new value loaded in this is called to start the output of that throttle value.
 * Parameters: ESC - Pointer to the single ESC_CONTROLLER that needs throttle to be updated.
 */
void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* ESC);
void ESC_SEND_CMD(ESC_CONTROLLER* ESC, uint32_t cmd, uint32_t motorNum);
void ESC_CALC_THROTTLE(ESC_CONTROLLER* escSet, RX_CONTROLLER* thisRX, uint8_t armed);


#define ONESHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = ((ADC_VALUE / 6.07) + 675))
#define MULTISHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = ((ADC_VALUE / 1.82) + 2250))
#define DSHOT_ADC_CONV(THROTTLE, ADC_VALUE) (THROTTLE = (ADC_VALUE - 1600))

#endif /* INC_ESC_H_ */
