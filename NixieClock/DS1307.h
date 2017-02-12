/*
 * DS1307.h: Communication with DS1307 over i2c
 *
 *
 * Created: 2/11/2017 9:10:52 AM
 * Author : Emmanuel Weber
 *
 * Copyright (c) 2017 Emmanuel Weber
 */ 

#ifndef _DS1307_H_
#define _DS1307_H_

///////////////////////////////////////////////////////////////////////////////
// DateTime: helper class to get time in and out of DS1307

class DateTime 
{
public:
    DateTime (uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour =0, uint8_t min =0, uint8_t sec =0,
				bool ap= false,bool apset = false );
    DateTime (const DateTime& copy);
    DateTime (const char* date, const char* time,bool ampmset=false);

    uint16_t year() const			{ return y; }
    uint8_t month() const			{ return m; }
    uint8_t day() const				{ return d; }
    uint8_t hour() const			{ return hh; }
    uint8_t minute() const			{ return mm; }
    uint8_t second() const			{ return ss; }
	bool   ampm() const				{ return mAmPm;}
	bool   ampmset() const			{ return mAmPmSet;}
	
protected:
    uint16_t y;
	uint8_t  m, d, hh, mm, ss;
	bool mAmPm;
	bool mAmPmSet;
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// RTC based on the DS1307 chip connected via I2C 

class RTC_DS1307 
{
public:
	enum Ds1307SqwPinMode
	{
		OFF				= 0x00,
		ON				= 0x80,
		SquareWave1HZ	= 0x10,
		SquareWave4kHz	= 0x11,
		SquareWave8kHz	= 0x12,
		SquareWave32kHz = 0x13
	};

	bool Init(bool ampm);
    static bool writeSqwPinMode(Ds1307SqwPinMode mode);

	static DateTime now(void);
    static void adjust(const DateTime& dt);

   
//    uint8_t readnvram(uint8_t address);
//    void readnvram(uint8_t* buf, uint8_t size, uint8_t address);
//    void writenvram(uint8_t address, uint8_t data);
//    void writenvram(uint8_t address, uint8_t* buf, uint8_t size);
	
};

//
///////////////////////////////////////////////////////////////////////////////


#endif // _DS1307_H_
