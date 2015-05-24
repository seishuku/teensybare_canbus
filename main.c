#include "MK20D7.h"
#include <sys/types.h>
#include <stdlib.h>
#include "spi.h"
#include "can.h"

// Helpful bits
#define BIT0             (0x0001)
#define BIT1             (0x0002)
#define BIT2             (0x0004)
#define BIT3             (0x0008)
#define BIT4             (0x0010)
#define BIT5             (0x0020)
#define BIT6             (0x0040)
#define BIT7             (0x0080)
#define BIT8             (0x0100)
#define BIT9             (0x0200)
#define BIT10            (0x0400)
#define BIT11            (0x0800)
#define BIT12            (0x1000)
#define BIT13            (0x2000)
#define BIT14            (0x4000)
#define BIT15            (0x8000)

// Timing junk
uint32_t Time=0;
volatile uint32_t SysTick=0;

// CANbus frame RX message
CAN_Frame_t rxmsg;

// cycle delay, useful for debugging
void delay(volatile unsigned i)
{
	while(i--);
}

// Display 4, 4 digit numbers across two display modules
void _display4(int16_t a, int16_t b, int16_t c, int16_t d)
{
	uint8_t _digit[]={ 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF }, i=0;
	uint8_t buf[16]={ 0xC0, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF };
	uint8_t aneg=0, bneg=0, cneg=0, dneg=0;

	if(a>=9999)	a=9999;
	if(a<=-999)	a=-999;
	if(a<0)	{	aneg=1;	a=abs(a);	}
	i=12;	while(a)	{	buf[i++]=_digit[a%10];	a/=10;	}
	if(aneg)	buf[i++]=0xBF;

	if(b>=9999)	b=9999;
	if(b<=-999)	b=-999;
	if(b<0)	{	bneg=1;	b=abs(b);	}
	i=8;	while(b)	{	buf[i++]=_digit[b%10];	b/=10;	}
	if(bneg)	buf[i++]=0xBF;

	if(c>=9999)	c=9999;
	if(c<=-999)	c=-999;
	if(c<0)	{	cneg=1;	c=abs(c);	}
	i=4;	while(c)	{	buf[i++]=_digit[c%10];	c/=10;	}
	if(cneg)	buf[i++]=0xBF;

	if(d>=9999)	d=9999;
	if(d<=-999)	d=-999;
	if(d<0)	{	dneg=1;	d=abs(d);	}
	i=0;	while(d)	{	buf[i++]=_digit[d%10];	d/=10;	}
	if(dneg)	buf[i++]=0xBF;

	GPIOC_PCOR|=BIT0;
	SPI_WriteBuf(buf, 16);
	GPIOC_PSOR|=BIT0;
}

// Display 2, 4 digit numbers on a single display
void _display2(int16_t a, int16_t b)
{
	uint8_t _digit[]={ 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF }, i=0;
	uint8_t buf[8]={ 0xC0, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF }, aneg=0, bneg=0;

	if(a>=9999)	a=9999;
	if(a<=-999)	a=-999;
	if(a<0)	{	aneg=1;	a=abs(a);	}
	i=4;	while(a)	{	buf[i++]=_digit[a%10];	a/=10;	}
	if(aneg)	buf[i++]=0xBF;

	if(b>=9999)	b=9999;
	if(b<=-999)	b=-999;
	if(b<0)	{	bneg=1;	b=abs(b);	}
	i=0;	while(b)	{	buf[i++]=_digit[b%10];	b/=10;	}
	if(bneg)	buf[i++]=0xBF;

	GPIOC_PCOR|=BIT0;
	SPI_WriteBuf(buf, 8);
	GPIOC_PSOR|=BIT0;
}

// Display one 8 digit number on a single display
void _display1(int32_t a)
{
	uint8_t _digit[]={ 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF }, i=0;
	uint8_t buf[8]={ 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, aneg=0;

	if(a>=99999999)	a=99999999;
	if(a<=-9999999)	a=-9999999;
	if(a<0)	{	aneg=1;	a=abs(a);	}
	while(a)	{	buf[i++]=_digit[a%10];	a/=10;	}
	if(aneg)	buf[i++]=0xBF;

	GPIOC_PCOR|=BIT0;
	SPI_WriteBuf(buf, 8);
	GPIOC_PSOR|=BIT0;
}

// Display nothing on two display modules
void _displayOff(void)
{
	uint8_t buf[8]={ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	GPIOC_PCOR|=BIT0;
	SPI_WriteBuf(buf, 8);
	SPI_WriteBuf(buf, 8);
	GPIOC_PSOR|=BIT0;
}

uint16_t RPM=0, CoolantTemp=0, MPH=0, FuelLevel=0;

int main(void)
{
	// Set up ports and pins

	// LED on pin 13 (port C5)
	PORTC_PCR5=PORT_PCR_MUX(1);
	GPIOC_PDDR|=BIT5;
	GPIOC_PCOR|=BIT5;

	// Chip select for the displays
	PORTC_PCR0=PORT_PCR_MUX(1);
	GPIOC_PDDR|=BIT0;
	GPIOC_PSOR|=BIT0;

	// Initalize the SPI module (used for 74HC595 displays)
	SPI_Init();

	// Initalize the CANbus module
	CAN_Init();

	// Inital set to clear any garbage
	_displayOff();

	// Inital time setting
	Time=SysTick;

	while(1)
	{
		// Flash the LED as a heartbeat indicator.
		if((SysTick-Time)>50)
		{
			Time=SysTick;
			GPIOC_PTOR|=BIT5;
		}

		// Traslate a few CAN messages into something useful...
		while(CAN_ReadFrame(&rxmsg))
		{
			// 0x3C3 - Steering status
			// Byte 0 - Steering angle H
			// Byte 1 - Steering angle L
			// Byte 2 - Steering torque H
			// Byte 3 - Steering torque L

			// 0x359 - Speed
			// Byte 1 - Speed H
			// Byte 2 - Speed L
			if(rxmsg.MessageID==0x359)
				MPH=((rxmsg.Data[2]<<8)+rxmsg.Data[1])/322;

			// 0x35B - Engine
			// Byte 1 - RPM H
			// Byte 2 - RPM L
			// Byte 3 - Coolant Temp
			// Byte 4 - Pedal status
			if(rxmsg.MessageID==0x35B) // Engine/pedal status
			{
				RPM=((rxmsg.Data[2]<<8)+rxmsg.Data[1])>>2; // RPM = ((HiByte*256)+LowByte)/4
				CoolantTemp=((((rxmsg.Data[3]-64)*6)>>3)*18+325)/10; // DegC = (Byte-64)*0.75
			}

			if(rxmsg.MessageID==0x621) // Gauge cluster data
				FuelLevel=(int)((((rxmsg.Data[3]&0x7F/*mask out low fuel light*/)<<8)/55)*100)>>8; // Fuel level % = (FuelRemainingInLiters / 55LiterTank) * 100 
		}

		// Set the data to the display
		_display4(RPM, CoolantTemp, MPH, FuelLevel);
//		_display4(8888, 8888, 8888, 8888); //Testorz
	}

	return 0;
}
