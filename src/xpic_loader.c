#include "xpic_loader.h"
#include "netx_io_areas.h"


/* The PRAM and DRAM is defined in the imported objects. */
extern const unsigned long _binary_xpic_pram_bin_start[];
extern const unsigned long _binary_xpic_pram_bin_end[];
extern const unsigned long _binary_xpic_dram_bin_start[];
extern const unsigned long _binary_xpic_dram_bin_end[];


int xpic_loader(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptXpicComDebugArea);
	HOSTDEF(ptXpicComRegsArea);
	int iResult;
	unsigned long ulValue;
	unsigned int uiCnt;
	const unsigned long *pulSrcCnt;
	const unsigned long *pulSrcEnd;
	unsigned long *pulDstCnt;


	/* Be pessimistic. */
	iResult = -1;

	/* Can the XPIC clock be enabled? */
	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= HOSTMSK(clock_enable0_mask_xpic0);
	if( ulValue!=0 )
	{
		/* Enable the XPIC clock. */
		ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
		ulValue |= HOSTMSK(clock_enable0_xpic0_wm);
		ulValue |= HOSTMSK(clock_enable0_xpic0);
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
		ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;

		/* Set the XPIC to "hold". */
		ptXpicComDebugArea->ulXpic_hold_pc = HOSTMSK(xpic_hold_pc_hold);

		/* Clear all hold reasons. */
		ulValue  = HOSTMSK(xpic_break_irq_raw_break0_irq);
		ulValue |= HOSTMSK(xpic_break_irq_raw_break1_irq);
		ulValue |= HOSTMSK(xpic_break_irq_raw_soft_break_irq);
		ulValue |= HOSTMSK(xpic_break_irq_raw_single_step_irq);
		ulValue |= HOSTMSK(xpic_break_irq_raw_misalignment_irq);
		ptXpicComDebugArea->ulXpic_break_irq_raw = ulValue;

		/* Hold XPIC and request reset. */
		ulValue  = HOSTMSK(xpic_hold_pc_hold);
		ulValue |= HOSTMSK(xpic_hold_pc_reset_xpic);
		ptXpicComDebugArea->ulXpic_hold_pc = ulValue;

		/* Wait for the reset to be finished. */
		uiCnt = 0x100;
		do
		{
			ulValue  = ptXpicComDebugArea->ulXpic_break_status;
			ulValue &= HOSTMSK(xpic_break_status_xpic_reset_status);
			if( ulValue!=0 )
			{
				break;
			}
			else
			{
				--uiCnt;
			}
		} while( uiCnt>0 );

		if( uiCnt>0 )
		{
			/* Release the reset request, engage bank control and select bank 1. */
			ulValue  = HOSTMSK(xpic_hold_pc_hold);
			ulValue |= HOSTMSK(xpic_hold_pc_bank_control);
			ulValue |= HOSTMSK(xpic_hold_pc_bank_select);
			ptXpicComDebugArea->ulXpic_hold_pc = ulValue;

			/* Reset the XPIC user registers. */
			ptXpicComRegsArea->aulXpic_usr[0] = 0U;
			ptXpicComRegsArea->aulXpic_usr[1] = 0U;
			ptXpicComRegsArea->aulXpic_usr[2] = 0U;
			ptXpicComRegsArea->aulXpic_usr[3] = 0U;
			ptXpicComRegsArea->aulXpic_usr[4] = 0U;

			/* Reset the XPIC work registers (bank 1). */
			ptXpicComRegsArea->aulXpic_r[0] = 0U;
			ptXpicComRegsArea->aulXpic_r[1] = 0U;
			ptXpicComRegsArea->aulXpic_r[2] = 0U;
			ptXpicComRegsArea->aulXpic_r[3] = 0U;
			ptXpicComRegsArea->aulXpic_r[4] = 0U;
			ptXpicComRegsArea->aulXpic_r[5] = 0U;
			ptXpicComRegsArea->aulXpic_r[6] = 0U;
			ptXpicComRegsArea->aulXpic_r[7] = 0U;

			/* Select bank 0. */
			ulValue  = HOSTMSK(xpic_hold_pc_hold);
			ulValue |= HOSTMSK(xpic_hold_pc_bank_control);
			ptXpicComDebugArea->ulXpic_hold_pc = ulValue;

			/* Reset the XPIC work registers (bank 0). */
			ptXpicComRegsArea->aulXpic_r[0] = 0U;
			ptXpicComRegsArea->aulXpic_r[1] = 0U;
			ptXpicComRegsArea->aulXpic_r[2] = 0U;
			ptXpicComRegsArea->aulXpic_r[3] = 0U;
			ptXpicComRegsArea->aulXpic_r[4] = 0U;
			ptXpicComRegsArea->aulXpic_r[5] = 0U;
			ptXpicComRegsArea->aulXpic_r[6] = 0U;

			/* Set the stack pointer to the top of the DRAM. */
			ptXpicComRegsArea->aulXpic_r[7] = 0x2000U;

			/* Release the bank control. */
			ulValue = HOSTMSK(xpic_hold_pc_hold);
			ptXpicComDebugArea->ulXpic_hold_pc = ulValue;

			/* Set the execute address. */
			ptXpicComRegsArea->ulXpic_pc = 0xfffffffcU;

			/* Load the PRAM data. */
			pulSrcCnt = _binary_xpic_pram_bin_start;
			pulSrcEnd = _binary_xpic_pram_bin_end;
			pulDstCnt = (unsigned long*)(HOSTADDR(xpic_com_pram));
			while( pulSrcCnt<pulSrcEnd )
			{
				*(pulDstCnt++) = *(pulSrcCnt++);
			}

			/* Load the DRAM data. */
			pulSrcCnt = _binary_xpic_dram_bin_start;
			pulSrcEnd = _binary_xpic_dram_bin_end;
			pulDstCnt = (unsigned long*)(HOSTADDR(xpic_com_dram));
			while( pulSrcCnt<pulSrcEnd )
			{
				*(pulDstCnt++) = *(pulSrcCnt++);
			}

			/* Start the XPIC. */
			ptXpicComDebugArea->ulXpic_hold_pc = 0U;

			/* All OK. */
			iResult = 0;
		}
	}

	return iResult;
}



void xpic_stop(void)
{
	HOSTDEF(ptXpicComDebugArea);


	/* Set the XPIC to "hold". */
	ptXpicComDebugArea->ulXpic_hold_pc = HOSTMSK(xpic_hold_pc_hold);
}
