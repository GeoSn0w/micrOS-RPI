#include "../coreOS/FrameBuffer/framebuffer.h"
#include "../coreOS/WorkSpace/workspace.h"
#include "kerneltypes.h"
#include "../coreOS/GPIO/delays.h"
#include "../coreOS/GPU/mailbox.h"
#include "../coreStorage/coreStorage.h"
#include "../coreStorage/coreStorage_types.h"
#include "coreHRNG.h"
#include "../coreOS/GPIO/gpio.h"

char* KERN_VER_STRING = "micrOS Kernel v1.0 ~ Tue June 14 2022 10:22 PDT / root:micrOS-RPI_CoreOS_DEVELOPMENT~arm64";

#define PM_RSTC         ((volatile unsigned int*)(MMIO_BASE+0x0010001c))
#define PM_RSTS         ((volatile unsigned int*)(MMIO_BASE+0x00100020))
#define PM_WDOG         ((volatile unsigned int*)(MMIO_BASE+0x00100024))
#define PM_WDOG_MAGIC   0x5a000000
#define PM_RSTC_FULLRST 0x00000020

kern_return_t initializeFrameBuffer(void);
kern_return_t micrOS_vanityPrint(void);
kern_return_t initializeCoreStorage(void);
kern_return_t initializeHardwareRandomNumberGenerator(void);
kern_return_t kRebootDevice(void);
kern_return_t kShutdownDevice(void);
kern_return_t corePowerManagement(corePowerManagement_cmd powerAction);
kern_return_t powerOnSelfTest(void);
unsigned long getCurrentExceptionLevel(void);

int main(){
    initializeFrameBuffer();
    microPrint("[*] Initializing text mode printer...");
    microPrint_NewLine();
    microPrint("[+] Successfully intiliazed!");
    microPrint_NewLine();
    presentWorkSpaceWithParameters();
    initializeCoreStorage();
    initializeHardwareRandomNumberGenerator();
    powerOnSelfTest();
    while (1);
}

kern_return_t initializeFrameBuffer(){
    if (micrOS_Framebuffer_Init() == 0) {
        micrOS_vanityPrint();
        microPrint("[*] Initializing FrameBuffer...");
        microPrint_NewLine();
        microPrint("[+] FrameBuffer is at Address 0x"); microPrint_Hex(frameBufferAddress);
        microPrint_NewLine();
        return KERN_SUCCESS;
    } else {
        return KERN_FAILURE;
    }
}

kern_return_t initializeCoreStorage(){
    microPrint("[i] Starting coreStorage Service...");
    microPrint_NewLine();
    if (coreStorage_initialize(INT_READ_RDY) == coreStorage_SUCCESS) {
        microPrint("[+] Successfully initialized MicroSD Card!");
        microPrint_NewLine();
    } else if (coreStorage_initialize(INT_READ_RDY) == coreStorage_TIMEOUT){
        microPrint("[!] Cannot initialize coreStorage Service. MicroSD Card access timeout.");
        microPrint_NewLine();
    } else if (coreStorage_initialize(INT_READ_RDY) == coreStorage_FAILURE){
        microPrint("[!] Cannot initialize coreStorage Service. MicroSD Card access failed.");
        microPrint_NewLine();
    }
    return KERN_SUCCESS;
}

kern_return_t micrOS_vanityPrint(){
    microPrint("micrOS v1.0 - Raspbery PI 3");
    microPrint_NewLine(); microPrint_NewLine(); microPrint_NewLine();
    microPrint(KERN_VER_STRING); microPrint_NewLine();
    return KERN_SUCCESS;
}

kern_return_t initializeHardwareRandomNumberGenerator(){
    microPrint("[i] MMIO Base is at: 0x");microPrint_Hex(MMIO_BASE);
    microPrint_NewLine();
    microPrint("[i] Initializing Hardware Random Number Generator Engine...");
    microPrint_NewLine();
    unsigned int testResult = kernRandomize(0,100000);
    if (testResult != 0) {
        microPrint("[+] Successfully Hardware Random Number Generator Engine.");
        microPrint_NewLine();
        microPrint("[+] HRNG Test Result: 0x"); microPrint_Hex(testResult);
        microPrint_NewLine();
        return KERN_SUCCESS;
    } else {
        microPrint("[!] Hardware Random Number Generator Engine failed to initialize!");
        microPrint_NewLine();
        return KERN_FAILURE;
    }
}

kern_return_t corePowerManagement(corePowerManagement_cmd powerAction){
    switch (powerAction) {
        case 0xff :
            kShutdownDevice();
            break;
            
        case 0xfe :
            kRebootDevice();
            break;
    }
    return KERN_SUCCESS;
}

kern_return_t kShutdownDevice(){
    unsigned long r = 0;
        for(r=0;r<16;r++) {
            mailbox[0]=8*4;
            mailbox[1]=MBOX_REQUEST;
            mailbox[2]=MBOX_TAG_SETPOWER;
            mailbox[3]=8;
            mailbox[4]=8;
            mailbox[5]=(unsigned int)r;
            mailbox[6]=0;
            mailbox[7]=MBOX_TAG_LAST;
            mailbox_call(MBOX_CH_PROP);
        }
    
        // Power off all GPIO pins (except VCC)
        *GPFSEL0 = 0;
        *GPFSEL1 = 0;
        *GPFSEL2 = 0;
        *GPFSEL3 = 0;
        *GPFSEL4 = 0;
        *GPFSEL5 = 0;
        *GPPUD = 0;
        wait_cycles(150);
        *GPPUDCLK0 = 0xffffffff;
        *GPPUDCLK1 = 0xffffffff;
        wait_cycles(150);
        *GPPUDCLK0 = 0;
        *GPPUDCLK1 = 0;

        // power off the SoC (GPU + CPU)
        r = *PM_RSTS;
        r &= ~0xfffffaaa;
        r |= 0x555;
        *PM_RSTS = PM_WDOG_MAGIC | r;
        *PM_WDOG = PM_WDOG_MAGIC | 10;
        *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
    return KERN_SUCCESS;
}

kern_return_t kRebootDevice(){
    unsigned int r;
        r = *PM_RSTS; r &= ~0xfffffaaa;
        *PM_RSTS = PM_WDOG_MAGIC | r;   // boot from partition 0, thus rebooting the system.
        *PM_WDOG = PM_WDOG_MAGIC | 10;
        *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
    return KERN_SUCCESS;
}

kern_return_t powerOnSelfTest(){
    mailbox[0] = 8*4;
    mailbox[1] = MBOX_REQUEST;
    mailbox[2] = MBOX_TAG_GETSERIAL;
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 0;
    mailbox[6] = 0;
    mailbox[7] = MBOX_TAG_LAST;
    
    if (mailbox_call(MBOX_CH_PROP)){
        microPrint("[i] Board Serial Number is: "); microPrint_Hex(mailbox[6]); microPrint_Hex(mailbox[5]);
        microPrint_NewLine();
    } else {
        microPrint("[!] Board Serial Number cannot be determined!");
        microPrint_NewLine();
        return KERN_FAILURE;
    }
    
    unsigned long exceptionLvl = getCurrentExceptionLevel();
    microPrint("[i] Current Exception Level: EL: "); microPrint_Hex((exceptionLvl>>2)&3);
    return KERN_SUCCESS;
}

unsigned long getCurrentExceptionLevel(){
    unsigned long exceptionLevel;
    asm volatile ("mrs %0, CurrentEL" : "=r" (exceptionLevel));
    return exceptionLevel;
}
