#include "kernel.h"
#include "memoryManagement.h"
#include "core_process/process.h"
#include "../coreOS/FrameBuffer/framebuffer.h"
#include "../coreOS/WorkSpace/workspace.h"
#include "kerneltypes.h"
#include "../coreOS/GPIO/delays.h"
#include "../coreOS/GPU/mailbox.h"
#include "../coreStorage/coreStorage.h"
#include "../coreStorage/coreStorage_types.h"
#include "coreHRNG.h"
#include "../coreOS/GPIO/gpio.h"
#include "../coreOS/Common/coreCommon.h"
#include "../coreOS/Timer/timer.h"  // Include the timer header for SysTick initialization

kern_return_t printversionstring(void);

char* KERN_VER_STRING = "micrOS Kernel v1.0 ~ Tue June 14 2022 10:22 PDT / root:micrOS-RPI_CoreOS_DEVELOPMENT~arm64";

int main() {
    initializeFrameBuffer();
    initializeMemory();
    microPrint("[*] Initializing text mode printer...", true);
    microPrint("[+] Successfully initialized!", true);
    initializeCoreStorage();
    initializeHardwareRandomNumberGenerator();
    powerOnSelfTest();
    presentWorkSpaceWithParameters();

    // Initialize task management
    task_init();
    
    // Initialize timer
    timer_init();

    // Define stack sizes and create tasks
    uint32_t stack1[STACK_SIZE];
    uint32_t stack2[STACK_SIZE];
    task_create(task1, stack1, STACK_SIZE, 5);
    task_create(task2, stack2, STACK_SIZE, 10);

    // Start multitasking
    while (1) {
        task_yield();
    }
}

kern_return_t printversionstring() {
    microPrint(KERN_VER_STRING, true);
    return 0x0;
}

kern_return_t initializeMemory() {
    unsigned char *heap_init = micrOS_IntitializeHeap();
    int mmu_init = mobiledevice_initialize_MMU();

    int* testPointer;
    testPointer = (int*) malloc(3 * sizeof(int));

    if (testPointer == NULL) {
        microPrint("[!] Failed to allocate test memory region. Heap Failure.", true);
        microPrint("[!] Failed to successfully initialize memory management modules.", true);
    } else {
        microPrint("[+] Successfully allocated test memory region. Heap Success.", true);
        microPrint("[i] Heap begins at address 0x", false); microPrint_Hex(heap_init); microPrint_NewLine();
    }
    return KERN_FAILURE;
}

kern_return_t initializeFrameBuffer() {
    if (micrOS_Framebuffer_Init() == 0) {
        micrOS_vanityPrint();
        microPrint("[*] Initializing FrameBuffer...", true);
        microPrint("[+] FrameBuffer is at Address 0x", false); microPrint_Hex(frameBufferAddress);
        microPrint_NewLine();
        return KERN_SUCCESS;
    } else {
        return KERN_FAILURE;
    }
}

kern_return_t initializeCoreStorage() {
    microPrint("[i] Starting coreStorage Service...", true);
    if (coreStorage_initialize(INT_READ_RDY) == coreStorage_SUCCESS) {
        microPrint("[+] Successfully initialized MicroSD Card!", true);
    } else if (coreStorage_initialize(INT_READ_RDY) == coreStorage_TIMEOUT) {
        microPrint("[!] Cannot initialize coreStorage Service. MicroSD Card access timeout.", true);
    } else if (coreStorage_initialize(INT_READ_RDY) == coreStorage_FAILURE) {
        microPrint("[!] Cannot initialize coreStorage Service. MicroSD Card access failed.", true);
    }
    return KERN_SUCCESS;
}

kern_return_t micrOS_vanityPrint() {
    microPrint("micrOS v1.0 - Raspberry PI 3", false);
    microPrint_NewLine(); microPrint_NewLine(); microPrint_NewLine();
    microPrint(KERN_VER_STRING, true);
    return KERN_SUCCESS;
}

kern_return_t initializeHardwareRandomNumberGenerator() {
    microPrint("[i] MMIO Base is at: 0x", false); microPrint_Hex(MMIO_BASE);
    microPrint_NewLine();
    microPrint("[i] Initializing Hardware Random Number Generator Engine...", true);

    unsigned int testResult = kernRandomize(0, 100000);
    if (testResult != 0) {
        microPrint("[+] Successfully initialized Hardware Random Number Generator Engine.", true);
        microPrint("[+] HRNG Test Result: 0x", false); microPrint_Hex(testResult);
        microPrint_NewLine();
        return KERN_SUCCESS;
    } else {
        microPrint("[!] Hardware Random Number Generator Engine failed to initialize!", true);
        return KERN_FAILURE;
    }
}

kern_return_t corePowerManagement(corePowerManagement_cmd powerAction) {
    switch (powerAction) {
        case 0xff:
            kShutdownDevice();
            break;
        case 0xfe:
            kRebootDevice();
            break;
    }
    return KERN_SUCCESS;
}

kern_return_t kShutdownDevice() {
    unsigned long r = 0;
    for (r = 0; r < 16; r++) {
        mailbox[0] = 8 * 4;
        mailbox[1] = MBOX_REQUEST;
        mailbox[2] = MBOX_TAG_SETPOWER;
        mailbox[3] = 8;
        mailbox[4] = 8;
        mailbox[5] = (unsigned int)r;
        mailbox[6] = 0;
        mailbox[7] = MBOX_TAG_LAST;
        mailbox_call(MBOX_CH_PROP);
    }

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

    r = *PM_RSTS;
    r &= ~0xfffffaaa;
    r |= 0x555;
    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
    return KERN_SUCCESS;
}

kern_return_t kRebootDevice() {
    unsigned int r;
    r = *PM_RSTS;
    r &= ~0xfffffaaa;
    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
    return KERN_SUCCESS;
}

kern_return_t powerOnSelfTest() {
    mailbox[0] = 8 * 4;
    mailbox[1] = MBOX_REQUEST;
    mailbox[2] = MBOX_TAG_GETSERIAL;
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 0;
    mailbox[6] = 0;
    mailbox[7] = MBOX_TAG_LAST;

    if (mailbox_call(MBOX_CH_PROP)) {
        microPrint("[i] Board Serial Number is: ", false); microPrint_Hex(mailbox[6]); microPrint_Hex(mailbox[5]);
        microPrint_NewLine();
    } else {
        microPrint("[!] Board Serial Number cannot be determined!", true);
        return KERN_FAILURE;
    }

    unsigned long executionLvl = getCurrentExecutionLevel();
    microPrint("[i] Current Execution Level: EL: ", false); microPrint_Hex((executionLvl >> 2) & 3); microPrint_NewLine();
    return KERN_SUCCESS;
}

unsigned long getCurrentExecutionLevel() {
    unsigned long executionLevel;
    asm volatile ("mrs %0, CurrentEL" : "=r" (executionLevel));
    return executionLevel;
}

kern_return_t bootImageAtAddress(uint8_t address, int bootFlag) {
    asm volatile (
        "mov x0, x10;"
        "mov x1, x11;"
        "mov x2, x12;"
        "mov x3, x13;"
        "mov x30, 0x80000; ret"
    );
}

// Example task functions to test implementation

void task1(void) {
    while (1) {
        microPrint("Task 1 running...\n", true);
        task_yield();
    }
}

void task2(void) {
    while (1) {
        microPrint("Task 2 running...\n", true);
        task_yield();
    }
}
