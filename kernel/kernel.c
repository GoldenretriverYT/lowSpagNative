#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

void sugondeeznuts(){}

static void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

#define MEM_SIZE 32768
#define REGISTERS 16

uint8_t mem[MEM_SIZE] = { // hardcoded lowSpag binary
    0x22, 0x14, 0x00, 0x00, 0x48, 0x65, 0x6C, 0x6C,
    0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21,
    0x00, 0x00, 0x00, 0x00, 0x36, 0x04, 0x00, 0x00,
    0x33, 0x00, 0x01, 0x00, 0x31, 0x00, 0x00, 0x00,
    0x21, 0x00, 0x01, 0x00, 0x22, 0x2C, 0x00, 0x00,
    0x22, 0x38, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00,
    0x34, 0x00, 0x00, 0x00, 0x22, 0x18, 0x00, 0x00,
    0x33, 0x0D, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00,
    0x33, 0x0A, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00,
    0x22, 0x4C, 0x00, 0x00, 0x22, 0x4C, 0x00, 0x00
};
uint8_t registers[REGISTERS];

uint16_t memPtr = 0;
uint16_t pc = 0;

void memcpy(uint8_t *dest, uint8_t *src, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

void memset(uint8_t *dest, uint8_t val, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        dest[i] = val;
    }
}

unsigned countDigits(uint8_t x)
{
    int i = 1;
    while ((x /= 10) && ++i);
    return i;
}
unsigned getNumDigits(uint8_t x)
{
    x < 0 ? x = -x : 0;
    return
        x < 10 ? 1 :
        x < 100 ? 2 : countDigits(x);
}
#define tochar(x) '0' + x
void tostr(char* dest, uint8_t x)
{
    unsigned i = getNumDigits(x);
    char negative = x < 0;
    if (negative && (*dest = '-') & (x = -x) & i++);
    *(dest + i) = 0;
    while ((i > negative) && (*(dest + (--i)) = tochar(((x) % 10))) | (x /= 10));
}

unsigned countDigits16(uint16_t x)
{
    int i = 1;
    while ((x /= 10) && ++i);
    return i;
}
unsigned getNumDigits16(uint16_t x)
{
    x < 0 ? x = -x : 0;
    return
        x < 10 ? 1 :
        x < 100 ? 2 : countDigits16(x);
}
#define tochar(x) '0' + x
void tostr16(char* dest, uint16_t x)
{
    unsigned i = getNumDigits16(x);
    char negative = x < 0;
    if (negative && (*dest = '-') & (x = -x) & i++);
    *(dest + i) = 0;
    while ((i > negative) && (*(dest + (--i)) = tochar(((x) % 10))) | (x /= 10));
}

char* itoaBuf[32];

void itoa(int val, int base){
    memset(itoaBuf, 0, 32);
    int i = 30;

    for(; val && i ; --i, val /= base)
        itoaBuf[i] = "0123456789abcdef"[val % base];

}

int strlen(const char* str) {
    int len = 0;
    while(str[len] != '\0') {
        len++;
    }

    return len;
}

int strlenwithoffset(const char* str, int offset) {
    int len = 0;
    while(str[len+offset] != '\0') {
        len++;
    }

    return len;
}

int stroffset(const char* str, char c) {
    int len = 0;
    while(str[len] != c) {
        len++;
    }

    return len;
}

void cpuRun(struct limine_terminal *terminal);
void cpuCycle(struct limine_terminal *terminal);

void cpuRun(struct limine_terminal *terminal) {
    while(1) {
        if(pc+4 >= MEM_SIZE) {
            terminal_request.response->write(terminal, "done: pc out of bounds\n", 22);
            break;
        }

        cpuCycle(terminal);
    }
}

void cpuCycle(struct limine_terminal *terminal) {
    uint8_t opcode = mem[pc];

    /*if(opcode != 0x00) {
        terminal_request.response->write(terminal, "debug: running opcode ", 22);

        tostr(&itoaBuf, opcode);

        terminal_request.response->write(terminal, itoaBuf, strlen(itoaBuf));
    }*/

    uint8_t data[4] = {mem[pc], mem[pc+1], mem[pc+2], mem[pc+3]};

    switch(opcode) {
        case(0x00):
            // NOP
            pc += 4;
            break;
        
        // arithmetics 0x1?
        case(0x10):
            // add
            registers[0xF] = registers[data[1]] + registers[data[2]];
            pc += 4;
            break;
        case(0x11):
            // sub
            registers[0xF] = registers[data[1]] - registers[data[2]];
            pc += 4;
            break;
        case(0x13):
            // div
            registers[0xF] = registers[data[1]] * registers[data[2]];
            pc += 4;
            break;
        case(0x14):
            // mul
            registers[0xF] = registers[data[1]] / registers[data[2]];
            pc += 4;
            break;
        case(0x15):
            // mod
            registers[0xF] = registers[data[1]] % registers[data[2]];
            pc += 4;
            break;
        
        // flow 0x2?
        case(0x20):
            // jmpiz
            // not implemented!
            terminal_request.response->write(terminal, "warn: jmpiz not implemented!", 28);
            pc += 4;
            break;
        case 0x21:
            // skpequ
            if(registers[data[1]] == registers[data[2]]) {
                pc += 8;
            } else {
                pc += 4;
            }

            break;
        case 0x22:
            // jmp
            sugondeeznuts(); // satisfy the compiler
            //terminal_request.response->write(terminal, "debug: jmp to ", 14);
            // little endian
            uint16_t newAddr = data[1] | (data[2] << 8); 
            //tostr16(&itoaBuf, newAddr);
            //terminal_request.response->write(terminal, itoaBuf, strlen(itoaBuf));

            pc = newAddr;
            break;
        
        // data 0x3?

        case 0x30:
            // str (stores reg[arg1] to mem[memPtr])
            mem[memPtr] = registers[data[1]];
            pc += 4;
            break;
        case 0x31:
            // ld
            registers[data[1]] = mem[memPtr];
            pc += 4;
            break;
        case 0x32:
            // memstr (stors arg[1] to mem[memPtr])
            mem[memPtr] = data[1];
            pc += 4;
            break;
        case 0x33:
            // strbyte (stores arg[1] to reg[arg[2]])
            registers[data[2]] = data[1];
            pc += 4;
            break;
        case 0x34:
            // mptr_inc
            memPtr++;
            pc += 4;
            break;
        case 0x35:
            // mptr_dec
            memPtr--;
            pc += 4;
            break;
        case 0x36:
            // mptr_set (sets memPtr to (uint16_t)arg[1] << 8 | arg[2])
            // little endian
            memPtr = data[1] | (data[2] << 8);
            pc += 4;
            break;
        case 0x37:
            // mptr_setreg (sets memPtr from reg[arg[1]] << 8 | reg[arg[2]])
            // little endian
            memPtr = registers[data[1]] | (registers[data[2]] << 8);
            pc += 4;
            break;

        // special 0x7?, 0x8?
        case 0x70:
            // printn (prints reg[arg[1]] as number)
            terminal_request.response->write(terminal, "warn: printn not implemented!", 29);
            pc += 4;
            break;
        case 0x71:
            // printa (prints reg[arg[1]] as ascii)
            terminal_request.response->write(terminal, (char*)&registers[data[1]], 1);
            pc += 4;
            break;
    }
}

// The following will be our kernel's entry point.
void _start(void) {
    // Ensure we got a terminal
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        done();
    }

    // We should now be able to call the Limine terminal to print out
    // a simple "Hello World" to screen.
    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, "Running lowSpag program now!\r\n", 30);
    
    cpuRun(terminal);

    terminal_request.response->write(terminal, "Done!\r\n", 7);

    // We're done, just hang...
    done();
}
