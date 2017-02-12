/*
 * i2c.h: Functions for i2c communication
 *
 * Description: Very primitive functions to handle master i2c communication
 *              without interrupts.
 *
 * Created: 2/11/2017 9:10:52 AM
 * Author : Emmanuel Weber
 *
 * Copyright (c) 2017 Emmanuel Weber
 */ 

void i2cInit(void);						// Initialize i2c (assumes 16MHZ clock)
void i2cStart(void);					// Sends a start condition (sets TWSTA)
void i2cStop(void);						// Sends a stop condition (sets TWSTO)
bool i2cWrite(uint8_t data);			// Write one byte of data
bool i2cRead(uint8_t *data,bool ack);   // Read one byte of data
