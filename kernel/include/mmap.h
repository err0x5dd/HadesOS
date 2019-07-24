#ifndef MMAP_H
#define MMAP_H

#define MAP_VMM_CONTEXT         0x00400000
#define MAP_VMM_PAGEDIR         0x00401000
#define MAP_VMM_PAGETABLE       0x00402000
//                            - 0x00802fff
#define MAP_VMM_TEMP_CONTEXT    0x00803000
#define MAP_VMM_TEMP_PAGEDIR    0x00804000
#define MAP_VMM_TEMP_PAGETABLE  0x00805000

#define MAP_VMM_KSPACE_START    0x00a00000
#define MAP_VMM_KSPACE_END      0x00fff000

#define MAP_VMM_STACK           0xfffff000
#define MAP_VMM_USERSTACK       0xffffe000

#endif
