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
#include "main.h"

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

#define DSHOT_MIN_THROTTLE	47
#define DSHOT_MIN_IDLE		250
#define DSHOT_MAX_THROTTLE 	2047
#define XYZ_NEUTRAL_VALUE	1028
#define SENSITIVITY_CONST	0.25

#if defined(DSHOT150) || defined(DSHOT300) || defined(DSHOT600) || defined(DSHOT1200)

#define __DSHOT_CONSUME_BIT(__DSHOT_BYTE__, __BIT__) (__DSHOT_BYTE__ = (((__BIT__ & 0b1) == 0b1) ? DSHOT_HIGH_BIT : DSHOT_LOW_BIT))

/* Function Summary: Initiate the Electronic Speed Controller (ESC) for
 * a particular timer and DMA streams
 * Param: * dmaTickTimers - Pointer to predefined timer used to trigger dma streams
 * Param: * pwmTimer - Pointer to timer handle responsible for outputting to ESCs
 * Param: ** dmaHandlers - Array containing set of DMA streams
 * Return: Pointer to struct containing all necessary data for ESC operation
 */
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
		escSet->Throttle[i] = 0;
		for (int j = 0; j < DSHOT_PACKET_SIZE; j++) escSet->ThrottleDshot[i][j] = 0;
		escSet->Channel[i] = 4*i;
		escSet->SendingFlag = 0;
		escSet->Timer[i] = pwmTimer;
		escSet->DMA[i] = dmaHandlers[i];
 		escSet->CCR[i] = &(pwmTimer->Instance->CCR1) + i;
		*escSet->CCR[i] = 0;
	}
	for (int i = 0; i < ESC_COUNT; i++)
	{
		HAL_TIM_PWM_Start(pwmTimer, escSet->Channel[i]);
		HAL_DMA_RegisterCallback(escSet->DMA[i], HAL_DMA_XFER_HALFCPLT_CB_ID, *DMA_XferCpltCallback);
		HAL_DMA_Start_IT(escSet->DMA[i], (uint32_t) &escSet->ThrottleDshot[i],
								(uint32_t) escSet->CCR[i], DSHOT_PACKET_SIZE);
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


void DSHOT_SEND_PACKET(ESC_CONTROLLER* escSet, uint32_t data, uint32_t telemBit, uint32_t motorNum)
{
	uint16_t dshotBytes = makeDshotPacketBytes(data, telemBit);
	// 17th bit is to set CCR to 0 to keep it low between packets
	uint32_t dshotPacket[DSHOT_PACKET_SIZE] = {0};
	// Populate checksum bits
	for (int i = 18; i >= 3; i--)
	{
		__DSHOT_CONSUME_BIT(dshotPacket[i], dshotBytes);
		dshotBytes >>= 1;
	}
	escSet->SendingFlag = 1;
	switch(motorNum) {
		case (FRONT_LEFT_MOTOR):
			memcpy(escSet->ThrottleDshot[0], dshotPacket, sizeof(dshotPacket));
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[0], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			escSet->DMA[0]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_ENABLE(escSet->DMA[0]);
			break;
		case (FRONT_RIGHT_MOTOR):
			memcpy(escSet->ThrottleDshot[1], dshotPacket, sizeof(dshotPacket));
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[1], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			escSet->DMA[1]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_ENABLE(escSet->DMA[1]);
			break;
		case (BACK_LEFT_MOTOR):
			memcpy(escSet->ThrottleDshot[2], dshotPacket, sizeof(dshotPacket));
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[2], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			escSet->DMA[2]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_ENABLE(escSet->DMA[2]);
			break;
		case (BACK_RIGHT_MOTOR):
			memcpy(escSet->ThrottleDshot[3], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[3]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[3], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_ENABLE(escSet->DMA[3]);
			break;
		case (LEFT_SIDE_MOTORS):
			memcpy(escSet->ThrottleDshot[0], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[2], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[0]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[2]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[0], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[2], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			__HAL_DMA_ENABLE(escSet->DMA[0]);
			__HAL_DMA_ENABLE(escSet->DMA[2]);
			break;
		case (RIGHT_SIDE_MOTORS):
			memcpy(escSet->ThrottleDshot[1], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[3], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[1]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[3]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[1], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7) | DMA_FLAG_FEIF3_7;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[3], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_ENABLE(escSet->DMA[1]);
			__HAL_DMA_ENABLE(escSet->DMA[3]);
			break;
		case (FRONT_SIDE_MOTORS):
			memcpy(escSet->ThrottleDshot[0], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[1], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[0]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[1]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[0], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[1], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			__HAL_DMA_ENABLE(escSet->DMA[0]);
			__HAL_DMA_ENABLE(escSet->DMA[1]);
			break;
		case (BACK_SIDE_MOTORS):
			memcpy(escSet->ThrottleDshot[2], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[3], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[2]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[3]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[2], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[3], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_ENABLE(escSet->DMA[2]);
			__HAL_DMA_ENABLE(escSet->DMA[3]);
			break;
		case (ALL_MOTORS):
			memcpy(escSet->ThrottleDshot[0], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[1], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[2], dshotPacket, sizeof(dshotPacket));
			memcpy(escSet->ThrottleDshot[3], dshotPacket, sizeof(dshotPacket));
			escSet->DMA[0]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[1]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[2]->Instance->NDTR = DSHOT_PACKET_SIZE;
			escSet->DMA[3]->Instance->NDTR = DSHOT_PACKET_SIZE;
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[0], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[1], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[2], DMA_FLAG_TCIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_FEIF3_7);
			__HAL_DMA_CLEAR_FLAG(escSet->DMA[3], DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_FEIF0_4);
			__HAL_DMA_ENABLE(escSet->DMA[0]);
			__HAL_DMA_ENABLE(escSet->DMA[1]);
			__HAL_DMA_ENABLE(escSet->DMA[2]);
			__HAL_DMA_ENABLE(escSet->DMA[3]);
			break;
	}
	escSet->SendingFlag = 0;
}

/* Function Summary: Once the throttle has a new value loaded in this is called to
 * start the output of that throttle value.
 * Param: ESC - Pointer to the single ESC_CONTROLLER that needs throttle to be updated.
 * Return: VOID
 */
void ESC_UPDATE_THROTTLE(ESC_CONTROLLER* escSet)
{
	// Throttle cannot exceed 11 bits, so max value is 2047
	DSHOT_SEND_PACKET(escSet, escSet->Throttle[0], 0, FRONT_LEFT_MOTOR);
	DSHOT_SEND_PACKET(escSet, escSet->Throttle[1], 0, FRONT_RIGHT_MOTOR);
	DSHOT_SEND_PACKET(escSet, escSet->Throttle[2], 0, BACK_LEFT_MOTOR);
	DSHOT_SEND_PACKET(escSet, escSet->Throttle[3], 0, BACK_RIGHT_MOTOR);
}

// TO DO: Commands often times do not save, need to figure out why.
/* Function Summary: Send particular DSHOT command to ESC
 * Param: escSet - Pointer to the single ESC_CONTROLLER,
 * Param: cmd - command from available command list,
 * Param: motorNum - specific motor(s) to send the command to
 * Return: VOID
 */
void ESC_SEND_CMD(ESC_CONTROLLER* escSet, uint32_t cmd, uint32_t motorNum)
{
	// Need to set telemetry bit to 1 if either of these commands are sent
	if (cmd == DSHOT_CMD_SPIN_DIRECTION_NORMAL || DSHOT_CMD_SPIN_DIRECTION_REVERSED ||
			   DSHOT_CMD_SPIN_DIRECTION_1 || DSHOT_CMD_SPIN_DIRECTION_2)
	{
		for (int i = 0; i < 100; i++)
		{
			DSHOT_SEND_PACKET(escSet, cmd, 1, motorNum);
		}
	}
	// Commands I do not want to be used right now
	else if (cmd == DSHOT_CMD_3D_MODE_OFF || DSHOT_CMD_3D_MODE_ON || DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF ||
					DSHOT_CMD_SILENT_MODE_ON_OFF || DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE||
					DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY || DSHOT_CMD_ESC_INFO ||
					DSHOT_CMD_LED3_ON || DSHOT_CMD_LED3_OFF)
	{
		// Do nothing because I don't want these commands being sent currently
	}
	else if (cmd == DSHOT_CMD_BEACON1 || DSHOT_CMD_BEACON2 || DSHOT_CMD_BEACON3 ||
					DSHOT_CMD_BEACON4 || DSHOT_CMD_BEACON5)
	{
		for (int i = 0; i < 100; i++)
		{
			DSHOT_SEND_PACKET(escSet, cmd, 1, motorNum);
		}
		HAL_Delay(1000);
	}
	else
	{
		for (int i = 0; i < 100; i++) DSHOT_SEND_PACKET(escSet, cmd, 1, motorNum);
	}
}

/* Function Summary: Calculates throttle values for all motors based on RX inputs
 * Param: escSet - Pointer to the single ESC_CONTROLLER
 * Param: thisRX - data struct containing RX inputs
 * Param: armed - Need to tell if the arm switch is on or off, if off then throttle = DSHOT_MIN_IDLE
 * Return: VOID
 */
void ESC_CALC_THROTTLE(ESC_CONTROLLER* escSet, RX_CONTROLLER* thisRX, uint8_t armed)
{
	if (armed)
	{
		// Initiate array containing set of throttle values for motors
		MOTOR_THROTTLES motorSet;
		// Set the base throttle of each motor to the throttle stick value
		motorSet.FrontLeft = thisRX->throttle;
		motorSet.FrontRight = thisRX->throttle;
		motorSet.BackLeft = thisRX->throttle;
		motorSet.BackRight = thisRX->throttle;
		// Back side increases throttle if pitching forward, front side decreases
		if (thisRX->pitch > XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (thisRX->pitch - XYZ_NEUTRAL_VALUE) * SENSITIVITY_CONST;
			if (motorSet.FrontLeft > addVal) motorSet.FrontLeft -= addVal;
			else motorSet.FrontLeft = 0;
			if (motorSet.FrontRight > addVal) motorSet.FrontRight  -= addVal;
			else motorSet.FrontRight  = 0;
			motorSet.BackLeft += addVal;
			motorSet.BackRight += addVal;
		}
		// Front side increases throttle pitching backward, back side decreases
		else if (thisRX->pitch < XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (XYZ_NEUTRAL_VALUE - thisRX->pitch) * SENSITIVITY_CONST;
			if (motorSet.BackLeft > addVal) motorSet.BackLeft -= addVal;
			else motorSet.BackLeft = 0;
			if (motorSet.BackRight > addVal) motorSet.BackRight  -= addVal;
			else motorSet.BackRight  = 0;
			motorSet.FrontLeft += addVal;
			motorSet.FrontRight += addVal;
		}
		// Left side increases throttle if pitching to the right, right side decreases
		if (thisRX->roll > XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (thisRX->roll - XYZ_NEUTRAL_VALUE) * SENSITIVITY_CONST;
			if (motorSet.FrontRight > addVal) motorSet.FrontRight -= addVal;
			else motorSet.FrontRight = 0;
			if (motorSet.BackRight > addVal) motorSet.BackRight  -= addVal;
			else motorSet.BackRight  = 0;
			motorSet.FrontLeft += addVal;
			motorSet.BackLeft += addVal;
		}
		// Right side increases throttle if pitching to the left, left side decreases
		else if (thisRX->roll < XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (XYZ_NEUTRAL_VALUE - thisRX->roll) * SENSITIVITY_CONST;
			if (motorSet.FrontLeft > addVal) motorSet.FrontLeft -= addVal;
			else motorSet.FrontLeft = 0;
			if (motorSet.BackLeft > addVal) motorSet.BackLeft  -= addVal;
			else motorSet.BackLeft  = 0;
			motorSet.FrontRight += addVal;
			motorSet.BackRight += addVal;
		}
		// Front left and back right (if spinning counter-clockwise) increase if yaw to the right
		if (thisRX->yaw > XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (thisRX->yaw - XYZ_NEUTRAL_VALUE) * SENSITIVITY_CONST;
			if (motorSet.FrontRight > addVal) motorSet.FrontRight -= addVal;
			else motorSet.FrontRight = 0;
			if (motorSet.BackLeft > addVal) motorSet.BackLeft  -= addVal;
			else motorSet.BackLeft  = 0;
			motorSet.FrontLeft += addVal;
			motorSet.BackRight += addVal;
		}
		// Front right and back left (if spinning clockwise) increase if yaw to the left
		else if (thisRX->yaw < XYZ_NEUTRAL_VALUE)
		{
			uint32_t addVal = (XYZ_NEUTRAL_VALUE - thisRX->yaw) * SENSITIVITY_CONST;
			if (motorSet.FrontLeft > addVal) motorSet.FrontLeft -= addVal;
			else motorSet.FrontLeft = 0;
			if (motorSet.BackRight > addVal) motorSet.BackRight  -= addVal;
			else motorSet.BackRight  = 0;
			motorSet.FrontRight += addVal;
			motorSet.BackLeft += addVal;
		}
		if (motorSet.FrontLeft > DSHOT_MAX_THROTTLE) motorSet.FrontLeft = DSHOT_MAX_THROTTLE;
		else if (motorSet.FrontLeft < DSHOT_MIN_IDLE) motorSet.FrontLeft = DSHOT_MIN_IDLE;
		if (motorSet.FrontRight > DSHOT_MAX_THROTTLE) motorSet.FrontRight = DSHOT_MAX_THROTTLE;
		else if (motorSet.FrontRight < DSHOT_MIN_IDLE) motorSet.FrontRight = DSHOT_MIN_IDLE;
		if (motorSet.BackLeft > DSHOT_MAX_THROTTLE) motorSet.BackLeft = DSHOT_MAX_THROTTLE;
		else if (motorSet.BackLeft < DSHOT_MIN_IDLE) motorSet.BackLeft = DSHOT_MIN_IDLE;
		if (motorSet.BackRight > DSHOT_MAX_THROTTLE) motorSet.BackRight = DSHOT_MAX_THROTTLE;
		else if (motorSet.BackRight < DSHOT_MIN_IDLE) motorSet.BackRight = DSHOT_MIN_IDLE;
		escSet->Throttle[0] = motorSet.FrontLeft;
		escSet->Throttle[1] = motorSet.FrontRight;
		escSet->Throttle[2] = motorSet.BackLeft;
		escSet->Throttle[3] = motorSet.BackRight;
	}
	else
	{
		escSet->Throttle[0] = 0;
		escSet->Throttle[1] = 0;
		escSet->Throttle[2] = 0;
		escSet->Throttle[3] = 0;
	}
}

#endif










