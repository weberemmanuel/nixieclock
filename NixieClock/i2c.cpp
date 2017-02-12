/*
 * i2c.cpp: Functions for i2c communication
 *
 * Description: Very primitive functions to handle master i2c communication
 *              without interrupts.
 *
 * Created: 2/11/2017 9:10:52 AM
 * Author : Emmanuel Weber
 *
 * Copyright (c) 2017 Emmanuel Weber
 */ 

#include <avr/io.h>
#include "i2c.h"

//-----------------------------------------------------------------------------
// Name: i2cInit()
// Desc: Initialize i2c communication
// Note: Setup the bandwidth (100Khz) assuming a clock of 16Mhz
// TODO: Use F_CPU as input
//       16MHz / (16+2*TWBR*1) ~= 100kHz (http://www.atmel.com/Images/Atmel-2564-Using-the-TWI-Module-as-I2C-Master_ApplicationNote_AVR315.pdf)
//-----------------------------------------------------------------------------

void i2cInit(void) 
{
	TWBR = 72;                      
	TWSR = 0;
	TWCR |= _BV(TWEN); // enable
}

//-----------------------------------------------------------------------------
// Name: i2cStart()
// Desc: 
//-----------------------------------------------------------------------------

void i2cStart(void) 
{
	TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTA));
	while(!(TWCR & _BV(TWINT)));
}

//-----------------------------------------------------------------------------
// Name: i2cStop()
// Desc: 
//-----------------------------------------------------------------------------

void i2cStop(void) 
{
	TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTO));
	while(TWCR & (_BV(TWSTO)));
}

//-----------------------------------------------------------------------------
// Name: i2cRead()
// Desc: Read one byte 
//-----------------------------------------------------------------------------

bool i2cRead(uint8_t *data,bool ack) 
{
	if(ack)
	{
		 //return ACK after reception
		TWCR|= _BV(TWEA);
	 }
	 else
	 {
		 //return NACK after reception
		 //Signals slave to stop giving more data
		 //usually used for last byte read.
		 TWCR&=(~_BV(TWEA));
	 }

	// Now enable Reception of data by clearing TWINT
	TWCR|=_BV(TWINT);

	//Wait till done
	while(!(TWCR & _BV(TWINT)));

	//Check status
	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
	{
		// Data received and ACK returned
		// Data received and NACK returned
		*data=TWDR;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: i2cWrite()
// Desc: Write one byte
//-----------------------------------------------------------------------------

bool i2cWrite(uint8_t data) 
{
	TWDR = data;
	TWCR = (_BV(TWINT) | _BV(TWEN));
	while(!(TWCR & _BV(TWINT)));

	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		// SLA+W Transmitted and ACK received
		// SLA+R Transmitted and ACK received
		// DATA Transmitted and ACK received
		return true;
	}
	return false;
}
