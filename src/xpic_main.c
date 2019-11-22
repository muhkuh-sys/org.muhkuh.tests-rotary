/***************************************************************************
 *   Copyright (C) 2019 by Christoph Thelen                                *
 *   doc_bacardi@users.sourceforge.net                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <string.h>

#include "netx_io_areas.h"


/*-------------------------------------------------------------------------*/


static HOSTDEF(ptXpicComDebugArea);
static HOSTDEF(ptGpioComArea);
static HOSTDEF(ptMmioCtrlArea);
static HOSTDEF(ptHifIoCtrlArea);



static void gpio_delay_ticks(unsigned long ulTicks)
{
	unsigned long ulValue;


	/* Stop the counter. */
	ptGpioComArea->ulGpio_counter0_ctrl = 0U;

	ptGpioComArea->ulGpio_counter0_max = ulTicks;
	ptGpioComArea->ulGpio_counter0_cnt = 0;

	/* Start the timer. */
	ulValue  = HOSTMSK(gpio_counter0_ctrl_run);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_once);
	ptGpioComArea->ulGpio_counter0_ctrl = ulValue;

	/* Wait until the timer is not running anymore. */
	do
	{
		ulValue  = ptGpioComArea->ulGpio_counter0_ctrl;
		ulValue &= HOSTMSK(gpio_counter0_ctrl_run);
	} while( ulValue!=0 );

	/* Stop the timer. */
	ptGpioComArea->ulGpio_counter0_ctrl = 0;
}



typedef enum PIO_STATE_ENUM
{
	PIO_STATE_In = 0,
	PIO_STATE_Out0 = 1,
	PIO_STATE_Out1 = 2
} PIO_STATE_T;



static void setPins(PIO_STATE_T tPinState0, PIO_STATE_T tPinState1)
{
	unsigned long ulOe;
	unsigned long ulOut;



	ulOe = 0;
	ulOut = 0;

	switch(tPinState0)
	{
	case PIO_STATE_In:
		break;

	case PIO_STATE_Out0:
		ulOe |= 1U << 8U;
		break;

	case PIO_STATE_Out1:
		ulOe |= 1U << 8U;
		ulOut |= 1U << 8U;
		break;
	}

	switch(tPinState1)
	{
	case PIO_STATE_In:
		break;

	case PIO_STATE_Out0:
		ulOe |= 1U << 12U;
		break;

	case PIO_STATE_Out1:
		ulOe |= 1U << 12U;
		ulOut |= 1U << 12U;
		break;
	}

	ptHifIoCtrlArea->aulHif_pio_out[0] = ulOut;
	ptHifIoCtrlArea->aulHif_pio_oe[0] = ulOe;
}



#define SPI_DELAY_TICKS 256

static unsigned long readRotary(void)
{
	unsigned long ulData;
	unsigned int uiCnt;
	unsigned long ulValue;


	/* Read the rotary switch. */
	setPins(PIO_STATE_Out0, PIO_STATE_Out0);
	gpio_delay_ticks(SPI_DELAY_TICKS);

	setPins(PIO_STATE_Out0, PIO_STATE_Out1);
	gpio_delay_ticks(SPI_DELAY_TICKS);

	ulData = 0;
	uiCnt = 0;
	do
	{
		/* Get the next bit. */
		ulValue = ptHifIoCtrlArea->aulHif_pio_in[0];
		ulValue >>= 9U;
		ulValue &= 1U;
		ulData <<= 1U;
		ulData |= ulValue;

		setPins(PIO_STATE_Out1, PIO_STATE_Out1);
		gpio_delay_ticks(SPI_DELAY_TICKS);

		setPins(PIO_STATE_Out0, PIO_STATE_Out1);
		gpio_delay_ticks(SPI_DELAY_TICKS);

		++uiCnt;
	} while( uiCnt<32 );

	/* Extract the 3 rotary switches. */
	ulData &= 0x00000fffU;
	/* Invert the bits. */
	ulData ^= 0x00000fffU;

	return ulData;
}




typedef struct ROTARY_STATE_STRUCT
{
	unsigned long ulMsk;
	unsigned int uiSrt;
	int iIsFinished;
	unsigned int uiIndexOfNextExpectedValue;
	unsigned long ulLastValue;
	unsigned int uiDebounceCounter;
	unsigned int uiDebounceBorder;
	const unsigned long *pulExpectedValues;
	unsigned int sizExpectedValues;
} ROTARY_STATE_T;



static void rotary_test_init(ROTARY_STATE_T *ptState, unsigned long ulMsk, unsigned int uiSrt, unsigned int uiDebounceBorder, const unsigned long *pulExpectedValues, unsigned int sizExpectedValues)
{
	ptState->ulMsk = ulMsk;
	ptState->uiSrt = uiSrt;
	ptState->iIsFinished = 0;
	ptState->uiIndexOfNextExpectedValue = 0;
	ptState->ulLastValue = 0xffffffffU;
	ptState->uiDebounceCounter = 0;
	ptState->uiDebounceBorder = uiDebounceBorder;
	ptState->pulExpectedValues = pulExpectedValues;
	ptState->sizExpectedValues = sizExpectedValues;
}



static int rotary_test_process(ROTARY_STATE_T *ptState, unsigned long ulValue)
{
	int iIsFinished;
	unsigned long ulLastValue;
	unsigned int uiDebounceCounter;
	unsigned int uiIndexOfNextExpectedValue;
	unsigned long ulExpectedValue;


	iIsFinished = ptState->iIsFinished;
	if( iIsFinished==0 )
	{
		ulValue  &= ptState->ulMsk;
		ulValue >>= ptState->uiSrt;

		/* Debounce the input. */
		ulLastValue = ptState->ulLastValue;
		uiDebounceCounter = ptState->uiDebounceCounter;
		if( ulLastValue==ulValue )
		{
			if( uiDebounceCounter<ptState->uiDebounceBorder )
			{
				++uiDebounceCounter;
			}
			else
			{
				uiDebounceCounter = 0;
				ulLastValue = ulValue;

				uiIndexOfNextExpectedValue = ptState->uiIndexOfNextExpectedValue;
				ulExpectedValue = ptState->pulExpectedValues[uiIndexOfNextExpectedValue];
				if( ulValue==ulExpectedValue )
				{
					++uiIndexOfNextExpectedValue;
					if( uiIndexOfNextExpectedValue<(ptState->sizExpectedValues) )
					{
						ptState->uiIndexOfNextExpectedValue = uiIndexOfNextExpectedValue;
					}
					else
					{
						iIsFinished = 1;
						ptState->iIsFinished = iIsFinished;
					}
				}
			}
		}
		else
		{
			uiDebounceCounter = 0;
			ulLastValue = ulValue;
		}
		ptState->ulLastValue = ulLastValue;
		ptState->uiDebounceCounter = uiDebounceCounter;
	}

	return iIsFinished;
}



static unsigned long rotary_test_get_status(ROTARY_STATE_T *ptState)
{
	unsigned long ulValue;


	ulValue = 0U;

	if( ptState->iIsFinished==0 )
	{
		ulValue = ptState->uiIndexOfNextExpectedValue;
	}
	else
	{
		ulValue = 0x80U;
	}

	return ulValue;
}



ROTARY_STATE_T atTests[3];

static const unsigned long aulExpcetedValues[16] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0
};


static void rotary_test(void)
{
	unsigned long ulValue;
	int iIsFinished;
	volatile unsigned long *pulStatus = (volatile unsigned long*)(0x00060000U);


	/* Setup the tests. */
	rotary_test_init(&(atTests[0]), 0x0000000fU, 0U, 4, aulExpcetedValues, sizeof(aulExpcetedValues)/sizeof(unsigned long));
	rotary_test_init(&(atTests[1]), 0x000000f0U, 4U, 4, aulExpcetedValues, sizeof(aulExpcetedValues)/sizeof(unsigned long));
	rotary_test_init(&(atTests[2]), 0x00000f00U, 8U, 4, aulExpcetedValues, sizeof(aulExpcetedValues)/sizeof(unsigned long));

	do
	{
		ulValue = readRotary();
		iIsFinished  = rotary_test_process(&(atTests[0]), ulValue);
		iIsFinished &= rotary_test_process(&(atTests[1]), ulValue);
		iIsFinished &= rotary_test_process(&(atTests[2]), ulValue);

		/* Combine the status in one DWORD. */
		ulValue  = rotary_test_get_status(&(atTests[0]));
		ulValue |= rotary_test_get_status(&(atTests[1])) <<  8U;
		ulValue |= rotary_test_get_status(&(atTests[2])) << 16U;
		pulStatus[0] = ulValue;
	} while( iIsFinished==0 );
}


/*-------------------------------------------------------------------------*/


void __attribute__ ((section (".init_code"))) test(void)
{
	rotary_test();

	/* That's all, stop the XPIC. */
	ptXpicComDebugArea->ulXpic_hold_pc = HOSTMSK(xpic_hold_pc_hold);

	/* Safety stop. */
	while(1);
}

/*-----------------------------------*/
