#ifndef MMAP_H
#define MMAP_H

#define MAP_VMM_CONTEXT        0x400000
#define MAP_VMM_PAGEDIR        0x401000
#define MAP_VMM_PAGETABLE      0x402000
//                           - 0x802FFF
#define MAP_VMM_TEMP_CONTEXT   0x803000
#define MAP_VMM_TEMP_PAGEDIR   0x804000
#define MAP_VMM_TEMP_PAGETABLE 0x805000

#define MAP_VMM_KSPACE_START   0xA00000
#define MAP_VMM_KSPACE_END     0xFFF000

#endif
