/*
 * ACCEL.c
 *
 *  Created on: Nov 11, 2020
 *      Author: Jeff Raines
 */

#include "main.h"
#include "ACCEL.h"



void ACCEL_INIT(I2C_HandleTypeDef* i2c)
{
	uint8_t txBuf = 0b1111111;
	HAL_I2C_Mem_Write_DMA(i2c, ACCEL_I2C_ADDR , FUNC_CFG_ACCESS, 0x1, &txBuf, 0x1);
	while(i2c->hdmatx->XferAbortCallback);
	i2c->State = HAL_I2C_STATE_READY;
	//HAL_I2C_Slave_R
}
