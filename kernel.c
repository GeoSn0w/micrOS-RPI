#include "kerneltypes.h"
#include "framebuffer.h"

int initializeFrameBuffer(void);

int main(){
    initializeFrameBuffer();
    micrOS_WriteLine(100,100,"micrOS v1.0 - Raspbery PI 3",0x0f);
    while (1);
}

int initializeFrameBuffer(){
    if (micrOS_Framebuffer_Init() == 0) {
        return KERN_SUCCESS;
    } else {
        return KERN_FAILURE;
    }
}
