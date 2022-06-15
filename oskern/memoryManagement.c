#include "memoryManagement.h"
#include "../coreOS/Common/coreCommon.h"
#include "../coreOS/GPIO/delays.h"
#include "../coreOS/GPIO/gpio.h"
#include "../coreOS/FrameBuffer/framebuffer.h"

#define PAGESIZE    4096
#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01        // 2M granule
#define PT_KERNEL   (0<<6)      // privileged, supervisor EL1 access only
#define PT_USER     (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW       (0<<7)      // read-write
#define PT_RO       (1<<7)      // read-only
#define PT_AF       (1<<10)     // accessed flag
#define PT_NX       (1UL<<54)   // no execute
#define PT_OSH      (2<<8)      // outter shareable
#define PT_ISH      (3<<8)      // inner shareable
#define PT_MEM      (0<<2)      // normal memory
#define PT_DEV      (1<<2)      // device MMIO
#define PT_NC       (2<<2)      // non-cachable

#define TTBR_CNP    1

extern volatile unsigned char _data;
extern volatile unsigned char _end;

// unsigned char *HEAP_START = &_end[0]; // End of kernel
unsigned char *HEAP_START = (unsigned char *)0x400000; // Top of stack
unsigned int   HEAP_SIZE  = 0x30000000; // Max heap size is 768Mb
unsigned char *HEAP_END;
static unsigned char *freeptr;
static unsigned int bytes_allocated = 0;
typedef unsigned char uint8_t;

void *micrOS_MemoryMove (void *destinationAddr, const void *sourceAddr, unsigned int length) {
    register unsigned int realDestination = (unsigned int)destinationAddr;
    register unsigned int realSource = (unsigned int)sourceAddr;

    if(!length)
        return destinationAddr;

    if (realDestination>realSource && realDestination<(realSource+length)) {
        while(length & 3)
        {
            length--;
            ((unsigned char *)realDestination)[length] = ((unsigned char *)realSource)[length];
        }

        while(length)
        {
            length-=4;
            *((unsigned int *)realDestination+length) = *((unsigned int *)realSource+length);
        }
    } else {
        while(length & 0xfffffffc)
        {
            *((unsigned int *)realDestination) = *((unsigned int *)realSource);
            realDestination+=4;
            realSource+=4;
            length-=4;
        }

        while(length) {
            *((unsigned char *)realDestination) = *((unsigned char *)realSource);
            realDestination++;
            realSource++;
            length--;
        }
    }
    return destinationAddr;
}

void *memset(void *dest, unsigned char val, unsigned short len){
    uint8_t *ptr = dest;
    while (len-- > 0)
       *ptr++ = val;
    return dest;
}

void *memcpy(void *dest, const void *src, unsigned short len){
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (len--)
       *d++ = *s++;
    return dest;
}

uint8_t memcmp(void *str1, void *str2, unsigned count){
    uint8_t *s1 = str1;
    uint8_t *s2 = str2;

    while (count-- > 0)
    {
       if (*s1++ != *s2++)
          return s1[-1] < s2[-1] ? -1 : 1;
    }

    return 0;
}

void mobiledevice_initialize_MMU(){
    microPrint("[i] MMU Init: Initializing Memory Management Unit (MMU)...", true);
    
    unsigned long data_page = (unsigned long)&_data/PAGESIZE;
    unsigned long r, b, *paging=(unsigned long*)&_end;
    
    microPrint("[Memory Management Unit] Info: Page Size is ", false); microPrint("4096", true);
    
    paging[0]=(unsigned long)((unsigned char*)&_end+2*PAGESIZE) |    // physical address
        PT_PAGE |     // it has the "Present" flag, which must be set, and we have area in it mapped by pages
        PT_AF |       // accessed flag. Without this we're going to have a Data Abort exception
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2, first 2M block
    paging[2*512]=(unsigned long)((unsigned char*)&_end+3*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2 2M blocks
    b=MMIO_BASE>>21;
    // skip 0th, as we're about to map it by L3
    for(r=1;r<512;r++)
        paging[2*512+r]=(unsigned long)((r<<21)) |  // physical address
        PT_BLOCK |    // map 2M block
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_USER |     // non-privileged
        (r>=b? PT_OSH|PT_DEV : PT_ISH|PT_MEM); // different attributes for device memory

    // identity L3
    for(r=0;r<512;r++)
        paging[3*512+r]=(unsigned long)(r*PAGESIZE) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        ((r<0x80||r>=data_page)? PT_RW|PT_NX : PT_RO); // different for code and data

    // TTBR1, kernel L1
    paging[512+511]=(unsigned long)((unsigned char*)&_end+4*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L2
    paging[4*512+511]=(unsigned long)((unsigned char*)&_end+5*PAGESIZE) |   // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L3
    paging[5*512]=(unsigned long)(MMIO_BASE+0x00201000) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_KERNEL |   // privileged
        PT_OSH |      // outter shareable
        PT_DEV;       // device memory

    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
    b=r&0xF;
    if(r&(0xF<<28)/*4k*/ || b<1/*36 bits*/) {
        microPrint("[Memory Management Unit] FAIL: 4k granule or 36 bit address space not supported.", true);
        return;
    }

    r=  (0xFF << 0) |    // AttrIdx=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8) |    // AttrIdx=1: device, nGnRE (must be OSH too)
        (0x44 <<16);     // AttrIdx=2: non cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (r));

    r=  (0b00LL << 37) | // TBI=0, no tagging
        (b << 32) |      // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 enable higher half
        (25LL   << 16) | // T1SZ=25, 3 levels (512G)
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 enable lower half
        (25LL   << 0);   // T0SZ=25, 3 levels (512G)
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((unsigned long)&_end + TTBR_CNP));
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((unsigned long)&_end + TTBR_CNP + PAGESIZE));
    microPrint("[Memory Management Unit] Info: Enabling Page Translation...", true);
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    r|=0xC00800;     // set mandatory reserved bits
    r&=~((1<<25) |   // clear EE, little endian translation tables
         (1<<24) |   // clear E0E
         (1<<19) |   // clear WXN
         (1<<12) |   // clear I, no instruction cache
         (1<<4) |    // clear SA0
         (1<<3) |    // clear SA
         (1<<2) |    // clear C, no cache at all
         (1<<1));    // clear A, no aligment check
    r|=  (1<<0);     // set M, enable MMU
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
    microPrint("[Memory Management Unit] MMU initialization complete.", true);
}

void *malloc(unsigned int size){
   if (size > 0) {
      void *allocated = freeptr;
      if ((long)allocated % 8 != 0) {
         allocated += 8 - ((long)allocated % 8);
      }
    
      if ((unsigned char *)(allocated + size) > HEAP_END) {
         return 0;
      } else {
         freeptr += size;
         bytes_allocated += size;

         return allocated;
      }
   }
   return 0;
}

unsigned char *micrOS_IntitializeHeap(){
    if ((long)HEAP_START % 8 != 0) {
        HEAP_START += 8 - ((long)HEAP_START % 8);
    }
    
    HEAP_END = (unsigned char *)(HEAP_START + HEAP_SIZE);
    freeptr = HEAP_START;
    return freeptr;
}
