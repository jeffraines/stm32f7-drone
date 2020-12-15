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

void XLG_INIT(I2C_HandleTypeDef* i2c)
{
	uint8_t writeThis = 0b10000000;
	XLG_WRITE(i2c, CTRL1_XL, &writeThis, 1);
	writeThis = 0b10001100;
	XLG_WRITE(i2c, CTRL2_G, &writeThis, 1);
}

void XLG_WRITE(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* writeByte, uint32_t writeSize)
{
	if (i2c->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Write_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, writeByte, writeSize);
		i2c->State = HAL_I2C_STATE_READY;
	}
}

void XLG_READ(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* readByte, uint32_t readSize)
{
	if (i2c->hdmarx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Read_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, readByte, readSize);
		i2c->State = HAL_I2C_STATE_READY;
	}
}

_Bool XLG_XL_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b1); // Mask with XLDA bit in STATUS_REG
}

_Bool XLG_G_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b10); // Mask with DGA bit in STATUS_REG
}

_Bool XLG_TEMP_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b100); // Mask with TDA bit in STATUS_REG
}

void XLG_G_DATA_READ(I2C_HandleTypeDef* i2c, XLG_DATA* gData)
{
	if (XLG_G_DATA_READY(i2c))
	{
		gData->dataReady = true;
		uint8_t readByte[6] = {0};
		XLG_READ(i2c, OUTX_H_G, &readByte[0], 1);
		XLG_READ(i2c, OUTX_L_G, &readByte[1], 1);
		XLG_READ(i2c, OUTY_H_G, &readByte[2], 1);
		XLG_READ(i2c, OUTY_L_G, &readByte[3], 1);
		XLG_READ(i2c, OUTZ_H_G, &readByte[4], 1);
		XLG_READ(i2c, OUTZ_L_G, &readByte[5], 1);
		gData->x = readByte[0];
		gData->x = (gData->x << 8) | readByte[1];
		gData->y = readByte[2];
		gData->y = (gData->y << 8) | readByte[3];
		gData->z = readByte[4];
		gData->z = (gData->z << 8) | readByte[5];
	}
	else
	{
		gData->dataReady = false;
	}
}

void XLG_XL_DATA_READ(I2C_HandleTypeDef* i2c, XLG_DATA* xlData)
{
	if (XLG_XL_DATA_READY(i2c))
	{
		xlData->dataReady = true;
		uint8_t readByte[6] = {0};
		XLG_READ(i2c, OUTX_H_XL, &readByte[0], 1);
		XLG_READ(i2c, OUTX_L_XL, &readByte[1], 1);
		XLG_READ(i2c, OUTY_H_XL, &readByte[2], 1);
		XLG_READ(i2c, OUTY_L_XL, &readByte[3], 1);
		XLG_READ(i2c, OUTZ_H_XL, &readByte[4], 1);
		XLG_READ(i2c, OUTZ_L_XL, &readByte[5], 1);
		xlData->x = readByte[0];
		xlData->x = (xlData->x << 8) | readByte[1];
		xlData->y = readByte[2];
		xlData->y = (xlData->y << 8) | readByte[3];
		xlData->z = readByte[4];
		xlData->z = (xlData->z << 8) | readByte[5];
	}
	else
	{
		xlData->dataReady = false;
	}
}
