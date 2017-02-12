/*
 * DS1307.cpp: Communication with DS1307 over i2c
 *
 *
 * Created: 2/11/2017 9:10:52 AM
 * Author : Emmanuel Weber
 *
 * Copyright (c) 2017 Emmanuel Weber
 */ 

 #include <avr/io.h>

#include "i2c.h"
#include "DS1307.h"

#define SECONDS_PER_DAY 86400L
#define SECONDS_FROM_1970_TO_2000 946684800

#define DS1307_ADDRESS		0x68
#define DS1307_SECONDS		0x00        // 0-59
#define DS1307_MINUTES		0x01		// 0-59
#define DS1307_HOURS		0x02		// 1-12 or 1-24
#define DS1307_DAYOFWEEK	0x03		// 1-7
#define DS1307_DAYS			0x04	
#define DS1307_MONTHS		0x05		// 1-12
#define DS1307_YEARS		0x06
#define DS1307_CONTROL		0x07
#define DS1307_NVRAM		0x08

#define DS1307_CH		7        // In seconds register
#define DS1307_AMPM     6        // In Hours register

#define DS1307_I2C_ADDRESS (0x68 << 1)

#define DS1307_BASEYEAR 2000

inline uint8_t toBCD(uint8_t val)
{
	return ((val/10)<<4)|(val%10);
}

static uint8_t conv2d(const char* p)
{
	uint8_t v = 0;
	if ('0' <= *p && *p <= '9')
	v = *p - '0';
	return 10 * v + *++p - '0';
}

//-----------------------------------------------------------------------------
// Name: DS1307Read()
// Desc: Read one byte from a DS1307 register
//-----------------------------------------------------------------------------

bool DS1307Read(uint8_t address,uint8_t *data)
{
	bool res;

	// Set start bit
	i2cStart();
	
	// SLA+W (for dummy write to set register pointer)
	res = i2cWrite(DS1307_I2C_ADDRESS + 0);	//DS1307 address + W
	if(!res)	return false;
	
	// Now send the address of required register
	res = i2cWrite(address);
	if(!res)	return false;
	
	//Repeat Start
	i2cStart();
	
	// SLA + R
	res = i2cWrite(DS1307_I2C_ADDRESS + 1);	//DS1307 Address + R
	if(!res)	return false;
	
	// Now read the value with NACK
	res = i2cRead(data,false);
	if(!res)	return false;
	
	// Set stop bit
	i2cStop();
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: DS1307Write()
// Desc: Write one byte in a DS1307 register
//-----------------------------------------------------------------------------

bool DS1307Write(uint8_t address,uint8_t data)
{
	bool res;	//result
	
	//Start
	i2cStart();
	
	//SLA+W
	res = i2cWrite(DS1307_I2C_ADDRESS + 0);	//DS1307 address + W
	if(!res)	return false;
	
	//Now send the address of required register
	res = i2cWrite(address);
	if(!res)	return false;
	
	//Now write the value
	res = i2cWrite(data);
	if(!res)	return false;
	
	i2cStop();
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//						DateTime implementation
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: DateTime()
// Desc: Consrutctor.
//-----------------------------------------------------------------------------

DateTime::DateTime (
	uint16_t year, 
	uint8_t  month, 
	uint8_t  day, 
	uint8_t  hour, 
	uint8_t  min, 
	uint8_t  sec,
	bool	 ap,
	bool	 apset)
	: y(year)
	, m(month)
	, d(day)
	, hh(hour)
	, mm(min)
	, ss(sec)
	, mAmPm(ap)
	, mAmPmSet(apset)
{
    
}

//-----------------------------------------------------------------------------
// Name: DateTime()
// Desc: Copy Consrutctor.
//-----------------------------------------------------------------------------

DateTime::DateTime (const DateTime& copy)
	: y(copy.y)
	, m(copy.m)
	, d(copy.d)
	, hh(copy.hh)
	, mm(copy.mm)
	, ss(copy.ss)
	, mAmPm(copy.mAmPm)
	, mAmPmSet(copy.mAmPmSet)
{
}

//-----------------------------------------------------------------------------
// Name: DateTime()
// Desc: Consrutctor.
// Note: For using "the compiler's time": DateTime now (__DATE__, __TIME__);
//       Sample input: date = "Feb 13 2001", time = "18:14:10"
//-----------------------------------------------------------------------------

DateTime::DateTime (const char* date, const char* time,bool ampmset) 
{
	// HACK: we only parse the last 2 digits of the year
    y = conv2d(date + 9) + DS1307_BASEYEAR;
   
    switch (date[0]) 
	{
        case 'J': m = date[1] == 'a' ? 1 : (date[2] == 'n') ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
		case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }

    d  = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);

	mAmPmSet = ampmset;
	if(ampmset)
	{
		if(hh > 12)
		{
			hh -=12;
			mAmPm = true;
		}
		else
		{
			mAmPm = false;
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
//						RTC_DS1307 implementation
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: Init()
// Desc: Initialize the DS1307.
//-----------------------------------------------------------------------------

bool RTC_DS1307::Init(bool ampm) 
{
	//Clear CH bit of RTC
	uint8_t temp;
	if(!DS1307Read(DS1307_SECONDS,&temp)) return false;

	//Clear CH Bit
	temp&=(~_BV(DS1307_CH));

	if(!DS1307Write(DS1307_SECONDS,temp)) return false;

	return true;
}

//-----------------------------------------------------------------------------
// Name: writeSqwPinMode()
// Desc: Update the Sqw Pin mode.
//-----------------------------------------------------------------------------

bool RTC_DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) 
{
	return DS1307Write(DS1307_CONTROL,mode);
}

//-----------------------------------------------------------------------------
// Name: now()
// Desc: Get the current time.
//-----------------------------------------------------------------------------

DateTime RTC_DS1307::now(void)
{
	uint8_t y=0,m=0,d=0,hh,mm,ss,temp;
	bool isAmPm, isAmPmSet;

	DS1307Read(DS1307_SECONDS,&temp);
	ss=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);

	DS1307Read(DS1307_MINUTES,&temp);
	mm=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);
	
	DS1307Read(DS1307_HOURS,&temp);
	if(temp & _BV(DS1307_AMPM) )
	{
		isAmPmSet = true;
		isAmPm    = (temp & 0b00001000) != 0;
		hh=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111);
	}
	else
	{
		isAmPmSet = false;
		isAmPm    = false;
		hh=(((temp & 0b00110000)>>4)*10)+(temp & 0b00001111);
	}

	DS1307Read(DS1307_DAYS,&temp);
	d=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111);

	DS1307Read(DS1307_MONTHS,&temp);
	m=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111);

	DS1307Read(DS1307_YEARS,&temp);
	y=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111) + DS1307_BASEYEAR;

	return DateTime (y, m, d, hh, mm, ss, isAmPm, isAmPmSet);
}

//-----------------------------------------------------------------------------
// Name: adjust()
// Desc: Update the current time.
//-----------------------------------------------------------------------------

void RTC_DS1307::adjust(const DateTime& dt) 
{
	DS1307Write(DS1307_SECONDS,toBCD(dt.second()));
	DS1307Write(DS1307_MINUTES,toBCD(dt.minute()));

	uint8_t hours = toBCD(dt.hour());
	if(dt.ampmset())
		hours |= _BV(DS1307_AMPM);
	DS1307Write(DS1307_HOURS,hours);
	
	DS1307Write(DS1307_DAYS,toBCD(dt.day()));
	DS1307Write(DS1307_MONTHS,toBCD(dt.month()));
	DS1307Write(DS1307_YEARS,toBCD(dt.year()+DS1307_BASEYEAR));
}

////////////////////////////////////////////////////////////////////////////////
