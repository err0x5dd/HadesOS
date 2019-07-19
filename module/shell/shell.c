#include "../../kernel/include/stdint.h"
#include "../../kernel/include/syscall.h"

#define PAGE_SIZE 4096

void print(char* str) {
    // request memory to store the string
    char* mem;
    asm volatile("mov %1, %%eax;"
                 "int $0x30;"
                 "mov %%eax, %0" : "=m" (mem) : "i" (SYSCALL_PAGE_ALLOC));
    
    char c = str[0];
    for(int i = 0; c != '\0' && i < PAGE_SIZE; i++) {
        mem[i] = str[i];
        c = str[i];
    }
    
    asm volatile("mov %0, %%eax;"
                 "mov %1, %%ebx;"
                 "int $0x30" : : "i" (SYSCALL_PRINTS), "m" (mem));
}

char getc(void) {
    char c;
    asm volatile("mov %1, %%eax;"
		 "int $0x30;"
		 "mov %%eax, %0" : "=r" (c) : "i" (SYSCALL_GETC));
    return c;
}

void* alloc(void) {
    uintptr_t mem;

    asm volatile("mov %1, %%eax;"
		 "int $0x30;"
		 "mov %%eax, %0" : "=m" (mem) : "i" (SYSCALL_PAGE_ALLOC));
    return mem;
}

//------------------------------------------------------------------------


const char* cmds[] = {"help"};

static void shell_cmds(int id);

static void _start(void) {
    //char* input = pmm_alloc();
    char* input = alloc();
    
    while(1) {
        
        uint8_t counter = 0;
        
        print("> ");
        // Get keyboard input
        input[counter] = getc();
        //switch_task();
        counter++;
        //kprint("input[%d]: %c\n", counter-1, input[counter-1]);
        print(input[counter-1]);
        while(input[counter-1] != '\n') {
            input[counter] = getc();
            //switch_task();
            counter++;
            //kprint("input[%d]: %c\n", counter-1, input[counter-1]);
            print(input[counter-1]);
    
            if(counter >= PAGE_SIZE) {
                print("INPUT CLEARED!\n");
                print("> ");
                counter = 0;
            }
        }
        input[counter-1] = '\0';
        
        if(input[0] == '\0') {
            continue;
        }
        
        int ret_code = 99;
        for(int i = 0; i < (sizeof(cmds) / 4); i++) {
            //kprint("Check if input of \"%s\" is command \"%s\"\n", input, cmds[i]);
            if(input[0] == cmds[i][0]) {
                // check for full command
                char c = ' ';
                for(int j = 0; c != '\0'; j++) {
                    c = cmds[i][j];
                    if(input[j] == c) {
                        ret_code = 0;
                        continue;
                    } else if (cmds[i][j] == '\0') {
                        break;
                    } else {
                        //kprint("Command is not %s\n", cmds[i]);
                        ret_code = 99;
                        break;
                    }
                }
                if(ret_code == 0) {
                    shell_cmds(i);
                    break;
                }
            }
        
        }
        if(ret_code == 99) {
            print("Command ");
	    print(input);
	    print(" not found!\n");
        }
        
    }
    print("INPUT STOPPED!\n");
    while(1);
}

static void shell_cmds(int id) {
    switch(id) {
    case 0:
        print("Available commands:\n");
        ////kprint("%d\n", (sizeof(cmds) / 4));
        for(int i = 0; i < (sizeof(cmds) / 4); i++) {
            print(cmds[i]);
        }
        break;
    }
}
