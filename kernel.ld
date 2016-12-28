/*  Bei _start soll die Ausfuehrung losgehen */
ENTRY(_start)

/*
 * Hier wird festgelegt, in welcher Reihenfolge welche Sektionen in die Binary
 * geschrieben werden sollen
 */
SECTIONS
{
    /*
     * . ist die aktuelle Position in der Datei
     * Kernel an 1 MB laden
     */
    . = 0x100000;

    /*
     * Erst der Multiboot-Header, damit dieser in den ersten 8 kiB ist
     * Die anderen Sektionen hintereinander
     */
    .text : {
        *(multiboot)
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
