#include "workspace_types.h"
#include "../Common/coreCommon.h"

kWorkSpaceFunc presentWorkSpaceWithParameters(){
    //micrOS_PaintRectangle(0, 0, 1024, 768, 0xff,1);
    setDefaultWallpaper();
    micrOS_PaintRectangle(0, 1040, 1920, 1080, 0x19,1);
    //micrOS_WriteLine(1650,50, "micrOS", 0x0f, 5);
    micrOS_PrintToScreen(1650,50, "micrOS");
    return WorkSpace_OK;
}
