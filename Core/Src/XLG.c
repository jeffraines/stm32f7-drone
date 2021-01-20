/*
 * XLG.c
 *
 *  Created on: Nov 11, 2020
 *      Author: Jeff Raines
 */

#include <XLG.h>

/* Function Summary: Starts the XLG chip and makes it read data at fastest rate
 * Param: * i2c is the predefined i2c handler
 * Return: VOID
 */
void XLG_INIT(I2C_HandleTypeDef* i2c)
{
	// Initiate accelerometer IC to start storing data
	uint8_t writeThis = 0b10000000;
	XLG_WRITE(i2c, CTRL1_XL, &writeThis, 1);
	// Initiate gyroscope IC to start storing data
	writeThis = 0b10001100;
	XLG_WRITE(i2c, CTRL2 _G, &writeThis, 1);
}

/* Function Summary: Writes to specific address on the IC
 * Param: * i2c - predefined i2c handler
 * Param: addr - address on IC chip,
 * Param: writeByte - data byte to write to addr
 * Param: writeSize - how many bytes to write
 * Return: VOID
 */
void XLG_WRITE(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* writeByte, uint32_t writeSize)
{
	if (i2c->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Write_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, writeByte, writeSize);
		i2c->State = HAL_I2C_STATE_READY;
	}
}

/* Function Summary: Reads from specific address on the IC
 * Param: * i2c - predefined i2c handler
 * Param: addr - address on IC chip,
 * Param: writeByte - data byte to read from addr
 * Param: writeSize - how many bytes to read
 * Return: VOID
 */
void XLG_READ(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* readByte, uint32_t readSize)
{
	if (i2c->hdmarx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Read_DMA(i2c, XLG_I2C_ADDR, addr, XLG_REG_SIZE, readByte, readSize);
		i2c->State = HAL_I2C_STATE_READY;
	}
}

/* Function Summary: Reads from interrupt pin on IC to see if accelerometer data is ready
 * Param: * i2c - predefined i2c handler
 * Return: VOID
 */
_Bool XLG_XL_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b1); // Mask with XLDA bit in STATUS_REG
}

/* Function Summary: Reads from interrupt pin on IC to see if gyroscope data is ready
 * Param: * i2c - predefined i2c handler
 * Return: VOID
 */
_Bool XLG_G_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b10); // Mask with DGA bit in STATUS_REG
}

/* Function Summary: Reads from interrupt pin on IC to see if temperature data is ready
 * Param: * i2c - predefined i2c handler
 * Return: VOID
 */
_Bool XLG_TEMP_DATA_READY(I2C_HandleTypeDef* i2c)
{
	uint8_t status = 0;
	XLG_READ(i2c, STATUS_REG, &status, 1);
	return (status & 0b100); // Mask with TDA bit in STATUS_REG
}

/* Function Summary: Reads from gyroscope data registers on IC to
 * Param: * i2c - predefined i2c handler
 * Param: * gData - pointer to structure olding 3-axis gyroscope data
 * Return: VOID
 */
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

/* Function Summary: Reads from accelerometer data registers on IC to
 * Param: * i2c - predefined i2c handler
 * Param: * xlData - pointer to structure olding 3-axis acceleromter data
 * Return: VOID
 */
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
