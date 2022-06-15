/*
 * Copyright (C) 2022 GeoSn0w (@FCE365)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "../Common/coreCommon.h"

int micrOS_Framebuffer_Init();
void micrOS_WriteLine(int x, int y, char *s, unsigned char attr, unsigned char foregroundColor, int zoom);
void microPrint(char *string, bool newLineRequired);
void microPrint_NewLine(void);
void microPrint_Character(unsigned char b);
void microPrint_Hex(unsigned int d);
void micrOS_PaintRectangle(int x1, int y1, int x2, int y2, unsigned char attr, int fill);
void micrOS_PrintToScreen(int x, int y, char *s);
void lfb_proprint(int x, int y, char *s);
void devicePaintPicture(char *picture_data, unsigned int picture_width, unsigned int picture_height);
extern unsigned char *frameBufferAddress;
int setDefaultWallpaper(void);
#endif /* FRAMEBUFFER_H */
