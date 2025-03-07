ENTRY(start)

SECTIONS
{
    /* Begin putting sections at 1 MB */
    . = 1M;

    /* Multiboot header first */
    .boot BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot_header))
    }

    /* Text section */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* Remove some unnecessary sections */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
}
