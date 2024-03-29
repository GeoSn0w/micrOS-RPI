#include "workspace_types.h"
#include "../Common/coreCommon.h"
kWorkSpaceFunc workspaceWindow_Constructor(int begin_x, int begin_y, int end_x, int end_y, unsigned char windowColor, char* windowTitle, bool isDismissable);
kWorkSpaceFunc presentWorkSpaceWithParameters(){
    //micrOS_PaintRectangle(0, 0, 1024, 768, 0xff,1);
    setDefaultWallpaper();
    micrOS_PaintRectangle(0, 1040, 1920, 1080, 0x19,1);
    //micrOS_WriteLine(1650,50, "micrOS", 0x0f, 5);
    micrOS_PrintToScreen(1650,50, "micrOS");
    workspaceWindow_Constructor(300,300, 1300, 900, 0x19, "micrOS - About", false);
    return WorkSpace_OK;
}

kWorkSpaceFunc workspaceWindow_Constructor(int begin_x, int begin_y, int end_x, int end_y, unsigned char windowColor, char* windowTitle, bool isDismissable){
    micrOS_PaintRectangle(begin_x, begin_y, end_x, end_y, windowColor, 1);
    micrOS_WriteLine(begin_x + 5, begin_y + 5, windowTitle, 0x0f, windowColor, 1);
}

kWorkSpaceFunc workspaceButton_Constructor(int begin_x, int begin_y, int end_x, int end_y, unsigned char buttonColor, char* buttonText){
    micrOS_PaintRectangle(begin_x, begin_y, end_x, end_y, buttonColor, 1);
    micrOS_WriteLine(begin_x + 5, begin_y + 5, buttonText, 0x0f, buttonColor, 1);
}

kWorkSpaceFunc createErrorWindow(char* errorMessage) {
    int windowWidth = 400;
    int windowHeight = 200;
    int windowX = (1920 - windowWidth) / 2;
    int windowY = (1080 - windowHeight) / 2; 
    unsigned char windowColor = 0x19;
    char* windowTitle = "Error";
    bool isDismissable = true;
    workspaceWindow_Constructor(windowX, windowY, windowX + windowWidth, windowY + windowHeight, windowColor, windowTitle, isDismissable);

    int textX = windowX + 20;
    int textY = windowY + 50;
    micrOS_WriteLine(textX, textY, errorMessage, 0x0f, windowColor, 1);
    int buttonX = windowX + windowWidth / 2 - 50;
    int buttonY = windowY + windowHeight - 40;
    unsigned char buttonColor = 0x0f;
    char* buttonText = "OK";
    workspaceButton_Constructor(buttonX, buttonY, buttonX + 100, buttonY + 30, buttonColor, buttonText);
    return WorkSpace_OK;
}
