#include "kerneltypes.h"
#include "../coreOS/FrameBuffer/framebuffer.h"
#include "../coreOS/WorkSpace/workspace.h"
#include "../coreStorage/coreStorage.h"
#include "../coreStorage/coreStorage_types.h"

int initializeFrameBuffer(void);
int micrOS_vanityPrint(void);
int initializeCoreStorage(void);

int main(){
    initializeFrameBuffer();
    micrOS_vanityPrint();
    microPrint("[*] Initializing text mode printer...");
    microPrint_NewLine();
    microPrint("[+] Successfully intiliazed!");
    microPrint_NewLine();
    presentWorkSpaceWithParameters();
    initializeCoreStorage();
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
}

int micrOS_vanityPrint(){
    microPrint("micrOS v1.0 - Raspbery PI 3");
    microPrint_NewLine();
    microPrint_NewLine();
    return KERN_SUCCESS;
}
