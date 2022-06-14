#include "kerneltypes.h"
#include "../coreOS/FrameBuffer/framebuffer.h"
#include "../coreOS/WorkSpace/workspace.h"
#include "../coreStorage/coreStorage.h"
#include "../coreStorage/coreStorage_types.h"
#include "coreHRNG.h"
#include "../coreOS/GPIO/gpio.h"

int initializeFrameBuffer(void);
int micrOS_vanityPrint(void);
int initializeCoreStorage(void);
int initializeHardwareRandomNumberGenerator(void);

int main(){
    initializeFrameBuffer();
    micrOS_vanityPrint();
   
    microPrint("[*] Initializing text mode printer...");
    microPrint_NewLine();
    microPrint("[+] Successfully intiliazed!");
    microPrint_NewLine();
    presentWorkSpaceWithParameters();
    initializeCoreStorage();
    initializeHardwareRandomNumberGenerator();
    
    while (1);
}

int initializeFrameBuffer(){
    if (micrOS_Framebuffer_Init() == 0) {
        return KERN_SUCCESS;
    } else {
        return KERN_FAILURE;
    }
}

int initializeCoreStorage(){
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

int micrOS_vanityPrint(){
    microPrint("micrOS v1.0 - Raspbery PI 3");
    microPrint_NewLine();
    microPrint_NewLine();
    return KERN_SUCCESS;
}

int initializeHardwareRandomNumberGenerator(){
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
