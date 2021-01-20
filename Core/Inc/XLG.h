/*
 * XLG.h
 *
 *  Created on: Nov 11, 2020
 *      Author: Jeff Raines
 */

#ifndef SRC_XLG_H_
#define SRC_XLG_H_

#include "main.h"
#include "stdbool.h"

/* 3-Axis Data Struct */
typedef struct XLG_DATA
{
	int16_t x;
	int16_t y;
	int16_t z;
	bool dataReady;
}XLG_DATA;

void XLG_INIT(I2C_HandleTypeDef* i2c);
void XLG_WRITE(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* writeByte, uint32_t writeSize);
void XLG_READ(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t* readByte, uint32_t readSize);
void XLG_G_DATA_READ(I2C_HandleTypeDef* i2c, XLG_DATA* gData);
void XLG_XL_DATA_READ(I2C_HandleTypeDef* i2c, XLG_DATA* xlData);

// I2C Address LSM6DS33
#define XLG_I2C_ADDR		0x6A << 1
// I2C Address LIS3MDL
#define M_I2C_ADDR			0x1D << 1
// Register size of XLG
#define XLG_REG_SIZE		0x1

/**************** LSM6DS33 Register Address Defines ****************/
// Embedded functions configuration register
#define FUNC_CFG_ACCESS		0x01

// FIFO configuration registers
#define FIFO_CTRL1			0x06	// FIFO threshold level setting
#define FIFO_CTRL2			0x07	// Pedo timestamp, when to write to FIFO, watermark flag
#define FIFO_CTRL3			0x08	// GYRO/XL decimation factors
#define FIFO_CTRL4			0x09	// MSByte memorization, Pedo decimation
#define FIFO_CTRL5			0x0A	// FIFO enable, FIFO speed, FIFO mode
#define ORIENT_CFG_G		0x0B	// Pitch, roll, yaw axis set

// INT1 pin control
#define INT1_CTRL			0x0D	// Set which interrupt is linked to INT1 pin

// INT2 pin control
#define INT2_CTRL			0x0E	// Set which interrupt is linked to INT2 pin

// Who I am ID register - read only
#define WHO_AM_I			0x0F	// Value fixed to 0x69h

// Accelerometer and gyroscope control registers
#define CTRL1_XL			0x10	// XL full-scale selection (+/- 2g maximum/default), anti-aliasing filter band, XL ODR setting
#define CTRL2_G				0x11	// GYRO full-scale selection (250 dps minimum/default), GYRO ODR setting
#define CTRL3_C				0x12	// Device settings: reboot, block data, INT pin mode, SPI/I2C, big/little endian, software reset
#define CTRL4_C				0x13	// Settings: XL bandwidth, GYRO sleep mode, INT signals, temp data on FIFO, data rdy mask, I2C disable, FIFO threshold use
#define CTRL5_C				0x14	// Rounding read from output reg (no rounding default), GYRO/XL self-test
#define CTRL6_C				0x15	// GYRO data edge/level/latch settings, XL high-performance mode disable
#define CTRL7_G				0x16	// GYRO high-performance mode disable, GYRO filter settings, source reg rounding func, GYRO high-pass filter cutoff freq
#define CTRL8_XL			0x17	// XL low-pass filter settings
#define CTRL9_XL			0x18 	// XL axis output enable (all enabled by default)
#define CTRL10_C			0x19	// GYRO axis output enable (all enabled by default), enable embedded functionality, reset pedo step, enable sign motion func

// Interrupts registers
#define WAKE_UP_SRC			0x1B	// Free-fall event detect, sleep event status, wake events detection
#define TAP_SRC				0x1C	// Tap event detections: single/double tap, tap sign, axis tap
#define D6D_SRC				0x1D	// Portrait, landscape, face-up and face-down events

// Status data register
#define STATUS_REG			0x1E 	// Temp/GYRO/XL data available

// Temperature output data register (Expressed as two's comp sign extended on the MSB)
#define OUT_TEMP_L			0x20	// (LSB) Lower 8 bits of total 16 bit temp value
#define OUT_TEMP_H			0x21	// Upper 8 bits of total 16 bit temp value

// Gyroscope output registers (Data expressed as 16-bit 2's complement)
#define OUTX_L_G			0x22 	// GYRO x-axis lower 8 bits (LSB)
#define OUTX_H_G			0x23 	// GYRO x-axis upper 8 bits (MSB)
#define OUTY_L_G			0x24	// GYRO y-axis lower 8 bits (LSB)
#define OUTY_H_G			0x25	// GYRO y-axis upper 8 bits (MSB)
#define OUTZ_L_G			0x26	// GYRO z-axis lower 8 bits (LSB)
#define OUTZ_H_G			0x27	// GYRO z-axis upper 8 bits (MSB)

// Accelerometer output regiters (Data expressed as 16-bit 2's complement)
#define OUTX_L_XL			0x28	// XL x-axis lower 8 bits (LSB)
#define OUTX_H_XL			0x29	// XL x-axis upper 8 bits (MSB)
#define OUTY_L_XL			0x2A	// XL y-axis lower 8 bits (LSB)
#define OUTY_H_XL			0x2B	// XL y-axis upper 8 bits (MSB)
#define OUTZ_L_XL			0x2C	// XL z-axis lower 8 bits (LSB)
#define OUTZ_H_XL			0x2D	// XL z-axis upper 8 bits (MSB)

// FIFO status registers
#define FIFO_STATUS1		0x3A 	// Number of unread words (16-bit axes) stored in FIFO
#define FIFO_STATUS2		0x3B	// FIFO watermark, overrun, full, empty, and num of unread words statuses
#define FIFO_STATUS3		0x3C	// Word of recursive pattern read at the next reading
#define FIFO_STATUS4		0x3D 	// Word of recursive pattern read at the next reading.

// FIFO data output registers (For proper reading, set BDU bit in CTRL3_C (0x12) to 1)
#define FIFO_DATA_OUT_L		0x3E	// FIFO data output (first byte) (LSB)
#define FIFO_DATA_OUT_H		0x3F	// FIFO data output (second byte) (MSB)

// Timestamp output registers (Expresse as 24-bit word, resolution set in WAKE_UP_DUR (0x5C))
#define TIMESTAMP0_REG		0x40	// TIMESTAMP first byte data output
#define TIMESTAMP1_REG		0x41	// TIMESTAMP second byte data output
#define TIMESTAMP2_REG		0x42	// TIMESTAMP third byte data output

// Step counter timestamp registers (When a step is detected, the value of TIMESTAMP_REG1 copied here)
#define STEP_TIMESTAMP_L	0x49	// Timestamp of last step detected lower 8 bits (LSB)
#define STEP_TIMESTAMP_H	0x4A	// Timestamp of last step detected upper 8 bits (MSB)

// Step counter output registers
#define STEP_COUNTER_L		0x4B	// Step counter output lower 8 bits	(LSB)
#define STEP_COUNTER_H		0x4C	// Step counter output upper 8 bits (MSB)

// Interrupt registers
#define FUNC_SRC			0x53	// Significant motion, tilt, step detector interrupt source register
#define TAP_CFG				0x58	// Timestamp, pedometer, tilt, filtering, and tap recognition functions configuration register
#define TAP_THS_6D			0x59	// Portrait/landscape position and tap function threshold register
#define INT_DUR2			0x5A	// Tap recognition function setting register
#define WAKE_UP_THS			0x5B	// Single and double-tap function threshold register
#define WAKE_UP_DUR			0x5C	// Free-fall, wakeup, timestamp and sleep mode functions duration setting register
#define FREE_FALL			0x5D	// Free-fall function duration setting register
#define MD1_CFG				0x5E	// Functions routing on INT1 register
#define MD2_CFG				0x5F	// Functions routing on INT2 register

/* Embedded Functions Registers: Accesible if FUNC_CFG_EN (1) in FUNC_CFG_ACCESS
  --------------------------------------------------------------------------------
    Modifications to EF Registers must be done while device in power-down mode!
    Accessing reserved registers could damage the device.
  -------------------------------------------------------------------------------- */
#define PEDO_THS_REG		0x0F	// Pedometer minimum threshold and internal full-scale configuration register
#define SM_THS				0x13	// Significant motion configuration register
#define PEDO_DEB_REG		0x14	// Pedometer debounce configuration register
#define STEP_COUNT_DELTA	0x15	// Time period register for step detection on delta time

#endif /* SRC_ACCEL_H_ */
