#include "kerneltypes.h"
#include "../coreOS/FrameBuffer/framebuffer.h"

int initializeFrameBuffer(void);
int micrOS_vanityPrint(void);

int main(){
    initializeFrameBuffer();
    micrOS_vanityPrint();
    microPrint("[*] Initializing text mode printer...");
    microPrint_NewLine();
    microPrint("[+] Successfully intiliazed!");
    microPrint_NewLine();
    while (1);
}

int initializeFrameBuffer(){
    if (micrOS_Framebuffer_Init() == 0) {
        return KERN_SUCCESS;
    } else {
        return KERN_FAILURE;
    }
}

int micrOS_vanityPrint(){
    microPrint("micrOS v1.0 - Raspbery PI 3");
    microPrint_NewLine();
    microPrint_NewLine();
    return KERN_SUCCESS;
}
