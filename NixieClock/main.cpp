/*
 * NixieClock.cpp
 *
 * Created: 2/11/2017 9:10:52 AM
 * Author : Emmanuel Weber
 *
 * Copyright (c) 2017 Emmanuel Weber
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "i2c.h"
#include "DS1307.h"

#define CLOCK_PIN  PIND2	// Pin to receive second ticks from RTC (external interrupt 0)
#define HOUR_PIN   PIND5	// Switch to change hours
#define MIN_PIN    PIND6	// Switch to change minutes
#define SEC_PIN    PIND7	// Switch to change seconds

volatile bool timeChange = false;
volatile uint8_t switchHistory = 0x00;

RTC_DS1307 rtc;

ISR(INT0_vect)
{
	sleep_disable();
}

ISR (PCINT2_vect)
{
	uint8_t changedBits = PIND ^ switchHistory;
	switchHistory = PIND;

	if(changedBits && (PIND & (HOUR_PIN|MIN_PIN|SEC_PIN)) )
	{
		timeChange = true;
		sleep_disable();
	}
}

//-----------------------------------------------------------------------------
// Name: initMCU()
// Desc: Init our micro controller
//-----------------------------------------------------------------------------

void initMCU(void) 
{
    sei();
	i2cInit();

	if (! rtc.Init(true))
	{
		//	TODO: do something useful ? Maybe show some errors on the nixies.
	}

// To set the time
//	rtc.adjust(DateTime(__DATE__, __TIME__,true));

	// Pull up resistor on RTC input
	PORTD |= (1<<PIND2); 

	// Configure input ports
	DDRD &= ~((1<<PIND2)|(1 << HOUR_PIN) | (1 << MIN_PIN) | (1 << SEC_PIN));
	
	rtc.writeSqwPinMode(RTC_DS1307::SquareWave1HZ);

	EICRA = (1<<ISC01)|(1<<ISC00);
	EIMSK = (1<<INT0);

	PCICR  |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT23) + (1 << PCINT22) + (1 << PCINT21);
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Main entry point.
//-----------------------------------------------------------------------------

int main(void)
{
	initMCU();

    while (1) 
    {
		DateTime now = rtc.now();
		if(timeChange)
		{
			timeChange = false;
			if(PIND & (1<<PIND5))
			{
				now = DateTime(now.year(),now.month(),now.day(),(now.hour()+1)%24,now.minute(),now.second());
			}
			if(PIND & (1<<PIND6))
			{
				now = DateTime(now.year(),now.month(),now.day(),now.hour(),(now.minute()+1)%60,now.second());
			}
			if(PIND & (1<<PIND7))
			{
				now = DateTime(now.year(),now.month(),now.day(),now.hour(),now.minute(),(now.second()+1)%60);
			}
			rtc.adjust(now);
		}
		
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		sleep_cpu();
    }
}

