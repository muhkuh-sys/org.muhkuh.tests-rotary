
MEMORY
{
	ASSERT_EMPTY(rwx)  : ORIGIN = 0x00000000, LENGTH = 0
	
	CODE(rx)           : ORIGIN = 0x00000000, LENGTH = 0x00002000
	DATA(rw)           : ORIGIN = 0x00000000, LENGTH = 0x00002000
}



SECTIONS
{
	.assert_empty (NOLOAD) :
	{
		/* NOTE: include all sections here which should be empty. */
		*(.data*)
		*(.trampolines)
	} >ASSERT_EMPTY


	.pram ORIGIN(CODE):
	{
		KEEP(*(.init_code))
		*(.text*)
	} >CODE


	/* NOTE: build the data section for the correct address 0, but locate it in the ELF file behind the code.
	 *       Without the relocation the ".code" and ".data" section would conflict as they have the same address.
	 */
	.dram ORIGIN(DATA) : AT(LENGTH(CODE))
	{
		*(.constants* .rodata*)
		*(.bss COMMON)
	} >DATA

	/* Place the stack at the end of the data section. */
	__STACK_TOP__ = ORIGIN(DATA) + LENGTH(DATA) ;
}



