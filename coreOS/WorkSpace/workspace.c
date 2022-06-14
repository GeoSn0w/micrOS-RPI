#include "../FrameBuffer/framebuffer.h"
#include "workspace_types.h"

kWorkSpaceFunc presentWorkSpaceWithParameters(){
    //micrOS_PaintRectangle(0, 0, 1024, 768, 0xff,1);
    micrOS_PaintRectangle(0, 735, 1022, 768, 0x19,1);
    return WorkSpace_OK;
}
