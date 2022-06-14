#include "../FrameBuffer/framebuffer.h"
#include "workspace_types.h"

kWorkSpaceFunc presentWorkSpaceWithParameters(){
    micrOS_PaintRectangle(0, 0, 301, 50, 0x0f, 0);
    return WorkSpace_OK;
}
