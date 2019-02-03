ENTRY(_start)

OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x600000;
    
    
    .text : {
        *(.text)
    }
    .data ALIGN(4096) : {
        *(.data)
    }
    .rodata ALIGN(4096) : {
        *(.rodata)
    }
    .bss ALIGN(4096) : {
        *(.bss)
    }
    
}
