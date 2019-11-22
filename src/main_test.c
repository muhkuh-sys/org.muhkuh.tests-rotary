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


#include "main_test.h"

#include <string.h>

#include "netx_io_areas.h"

#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"
#include "xpic_loader.h"


/*-------------------------------------------------------------------------*/


static unsigned long s_ulVerbose;

static void setup_pins(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	unsigned long ulValue;


	/* Disable SDRAM and DPM on HIF pins. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = HOSTDFLT(hif_io_cfg);

	/* Set HIF_D8, D9 and D12 to HIF function. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[8] = HOSTMMIO(PIO);
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[9] = HOSTMMIO(PIO);
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[12] = HOSTMMIO(PIO);

	/* Sample HIF inputs on every system clock. */
	ulValue  = HOSTMSK(hif_pio_cfg_filter_irqs);
	ulValue |= HOSTMSK(hif_pio_cfg_filter_en_in);
	ulValue |= 1U << HOSTSRT(hif_pio_cfg_in_ctrl);
	ptHifIoCtrlArea->ulHif_pio_cfg = ulValue;

	/* Set D8 and D12 to output 0, set D9 to input. */
	ptHifIoCtrlArea->aulHif_pio_out[0] = 0;
	ulValue = 1U << 8U;
	ulValue |= 1U << 12U;
	ptHifIoCtrlArea->aulHif_pio_oe[0] = ulValue;
}



TEST_RESULT_T test(TEST_PARAMETER_T *ptTestParams)
{
	TEST_RESULT_T tResult;
	int iResult;
	TEST_COMMAND_T tCommand;


	systime_init();

	/* Switch all LEDs off. */
	rdy_run_setLEDs(RDYRUN_OFF);

	s_ulVerbose = ptTestParams->ulVerbose;
	if( s_ulVerbose!=0 )
	{
		uprintf("\f. *** Rotary test by doc_bacardi@users.sourceforge.net ***\n");
		uprintf("V" VERSION_ALL "\n\n");

		/* Get the test parameter. */
		uprintf(". Parameters: 0x%08x\n", (unsigned long)ptTestParams);
		uprintf(".   Verbose: 0x%08x\n", ptTestParams->ulVerbose);
		uprintf(".   Command: 0x%08x\n", ptTestParams->ulCommand);
	}

	tResult = TEST_RESULT_ERROR;
	tCommand = (TEST_COMMAND_T)(ptTestParams->ulCommand);
	switch(tCommand)
	{
	case TEST_COMMAND_Init:
	case TEST_COMMAND_Stop:
		tResult = TEST_RESULT_OK;
		break;
	}
	if( tResult!=TEST_RESULT_OK )
	{
		uprintf("Invalid command: 0x%08x\n", ptTestParams->ulCommand);
	}
	else
	{
		switch(tCommand)
		{
		case TEST_COMMAND_Init:
			if( s_ulVerbose!=0 )
			{
				uprintf("Initializing XPIC system.\n");
			}

			setup_pins();

			/* Load the XPIC code. */
			iResult = xpic_loader();
			if( iResult!=0 )
			{
				if( s_ulVerbose!=0 )
				{
					uprintf("Failed to start the XPIC code.\n");
				}
				tResult = TEST_RESULT_ERROR;
			}
			else
			{
				if( s_ulVerbose!=0 )
				{
					uprintf("Init OK.\n");
				}
			}
			break;

		case TEST_COMMAND_Stop:
			if( s_ulVerbose!=0 )
			{
				uprintf("Stopping XPIC system.\n");
			}
			xpic_stop();
			break;
		}
	}

	/* Switch the SYS LED to green if the test was successful. Switch it
	 * to yellow if an error occurred.
	 */
	if( tResult==TEST_RESULT_OK )
	{
		rdy_run_setLEDs(RDYRUN_GREEN);
	}
	else
	{
		rdy_run_setLEDs(RDYRUN_YELLOW);
	}

	return tResult;
}


/*-----------------------------------*/
