/*
 * XLG.c
 *
 *  Created on: Nov 11, 2020
 *      Author: Jeff Raines
 */

#include <XLG.h>

/**
  * @brief
  * @note
  * @param i2c			write here
  * @param addr			write here
  * @param writeByte	write here
  * @param writeSize	write here
  * @retval void
  */
void XLG_WRITE(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* writeByte, uint32_t writeSize)
{
	HAL_I2C_Mem_Write_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, writeByte, writeSize);
	while(i2c->hdmatx->XferCpltCallback);
	i2c->State = HAL_I2C_STATE_READY;
}

void XLG_READ(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* readByte, uint32_t readSize)
{
	HAL_I2C_Mem_Read_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, readByte, readSize);
	while(i2c->hdmatx->XferCpltCallback);
	i2c->State = HAL_I2C_STATE_READY;
}

void XLG_G_DATA_READ(I2C_HandleTypeDef* i2c, uint8_t readByte[])
{
	XLG_READ(i2c, OUTX_L_G, readByte[0], 1);
	XLG_READ(i2c, OUTX_H_G, readByte[1], 1);
	XLG_READ(i2c, OUTY_L_G, readByte[2], 1);
	XLG_READ(i2c, OUTY_H_G, readByte[3], 1);
	XLG_READ(i2c, OUTZ_L_G, readByte[4], 1);
	XLG_READ(i2c, OUTZ_H_G, readByte[5], 1);
}
