/*
 * ACCEL.h
 *
 *  Created on: Nov 11, 2020
 *      Author: Jeff Raines
 */

#ifndef SRC_ACCEL_H_
#define SRC_ACCEL_H_

// I2C Address LSM6DS33
#define ACCEL_I2C_ADDR		0x6A << 1
// I2C Address LIS3MDL
#define MAG_I2C_ADDR		0x1D << 1

/* LSM6DS33 Register Address Defines */
// Embedded functions configurationr register
#define FUNC_CFG_ACCESS		0x01

// FIFO configuration registers
#define FIFO_CTRL1			0x06
#define FIFO_CTRL2			0x07
#define FIFO_CTRL3			0x08
#define FIFO_CTRL4			0x09
#define FIFO_CTRL5			0x0A
#define ORIENT_CFG_G		0x0B

// INT1 pin control
#define INT1_CTRL			0x0D

// INT2 pin control
#define INT2_CTRL			0x0E

// Who I am ID register - read only
#define WHO_AM_I			0x0F

// Accelerometer and gyroscope control registers
#define CTRL1_XL			0x10
#define CTRL2_G				0x11
#define CTRL3_C				0x12
#define CTRL4_C				0x13
#define CTRL5_C				0x14
#define CTRL6_C				0x15
#define CTRL7_G				0x16
#define CTRL8_XL			0x17
#define CTRL9_XL			0x18
#define CTRL10_C			0x19

// Interrupts registers
#define WAKE_UP_SRC			0x1B
#define TAP_SRC				0x1C
#define D6D_SRC				0x1D

// Status data register
#define STATUS_REG			0x1E

// Temperature output data register
#define OUT_TEMP_L			0x20
#define OUT_TEMP_H			0x21

// Gyroscope output registers
#define OUTX_L_G			0x22
#define OUTX_H_G			0x23
#define OUTY_L_G			0x24
#define OUTY_H_G			0x25
#define OUTZ_L_G			0x26
#define OUTZ_H_G			0x27

// Accelerometer output regiters
#define OUTX_L_XL			0x28
#define OUTX_H_XL			0x29
#define OUTY_L_XL			0x2A
#define OUTY_H_XL			0x2B
#define OUTZ_L_XL			0x2C
#define OUTZ_H_XL			0x2D

// FIFO status registers
#define FIFO_STATUS1		0x3A
#define FIFO_STATUS2		0x3B
#define FIFO_STATUS3		0x3C
#define FIFO_STATUS4		0x3D

// FIFO data output registers
#define FIFO_DATA_OUT_L		0x3E
#define FIFO_DATA_OUT_H		0x3F

// Timestamp output registers
#define TIMESTAMP0_REG		0x40
#define TIMESTAMP1_REG		0x41
#define TIMESTAMP2_REG		0x42

// Step counter timestamp registers
#define STEP_TIMESTAMP_L	0x49
#define STEP_TIMESTAMP_H	0x4A

// Step counter output registers
#define STEP_COUNTER_L		0x4B
#define STEP_COUNTER_H		0x4C

// Interrupt registers
#define FUNC_SRC			0x53
#define TAP_CFG				0x58
#define TAP_THS_6D			0x59
#define INT_DUR2			0x5A
#define WAKE_UP_THS			0x5B
#define WAKE_UP_DUR			0x5C
#define FREE_FALL			0x5D
#define MD1_CFG				0x5E
#define MD2_CFG				0x5F

/* Embedded Functions Registers: Accesible if FUNC_CFG_EN (1) in FUNC_CFG_ACCESS
  --------------------------------------------------------------------------------
    Modifications to EF Registers must be done while device in power-down mode!
    Accessing reserved registers could damage the device.
  -------------------------------------------------------------------------------- */
#define PEDO_THS_REG		0x0F
#define SM_THS				0x13
#define PEDO_DEB_REG		0x14
#define STEP_COUNT_DELTA	0x15



void ACCEL_INIT(I2C_HandleTypeDef* i2c);

#endif /* SRC_ACCEL_H_ */
