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

#include "mailbox.h"
#include "textmode.h"

unsigned int width, height, pitch, isrgb;   /* dimensions and channel order */
extern unsigned char *frameBufferAddress;
void microPrint_NewLine(void);
int current_Y = 0;
int current_X = 0;

int micrOS_Framebuffer_Init() {
    mailbox[0] = 35*4;
    mailbox[1] = MBOX_REQUEST;

    mailbox[2] = 0x48003;  //set phy wh
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 1024;         //FrameBufferInfo.width
    mailbox[6] = 768;          //FrameBufferInfo.height

    mailbox[7] = 0x48004;  //set virt wh
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = 1024;        //FrameBufferInfo.virtual_width
    mailbox[11] = 768;         //FrameBufferInfo.virtual_height

    mailbox[12] = 0x48009; //set virt offset
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0;           //FrameBufferInfo.x_offset
    mailbox[16] = 0;           //FrameBufferInfo.y.offset

    mailbox[17] = 0x48005; //set depth
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32;          //FrameBufferInfo.depth

    mailbox[21] = 0x48006; //set pixel order
    mailbox[22] = 4;
    mailbox[23] = 4;
    mailbox[24] = 1;           //RGB, not BGR preferably

    mailbox[25] = 0x40001; //get framebuffer, gets alignment on request
    mailbox[26] = 8;
    mailbox[27] = 8;
    mailbox[28] = 4096;        //FrameBufferInfo.pointer
    mailbox[29] = 0;           //FrameBufferInfo.size

    mailbox[30] = 0x40008; //get pitch
    mailbox[31] = 4;
    mailbox[32] = 4;
    mailbox[33] = 0;           //FrameBufferInfo.pitch

    mailbox[34] = MBOX_TAG_LAST;

    if(mailbox_call(MBOX_CH_PROP) && mailbox[20]==32 && mailbox[28]!=0) {
        mailbox[28]&=0x3FFFFFFF;   //convert GPU address to ARM address
        width = mailbox[5];          //get actual physical width
        height = mailbox[6];         //get actual physical height
        pitch = mailbox[33];         //get number of bytes per line
        isrgb = mailbox[24];         //get the actual channel order
        frameBufferAddress = (void*)((unsigned long)mailbox[28]);
        return 0;
    } else {
        return -1;
    }
}

void drawPixel(int x, int y, unsigned char attr) {
    int offs = (y * pitch) + (x * 4);
    *((unsigned int*)(frameBufferAddress + offs)) = vgapal[attr & 0x0f];
}


void drawChar(unsigned char ch, int x, int y, unsigned char attr, int zoom){
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=1;i<=(FONT_HEIGHT*zoom);i++) {
    for (int j=0;j<(FONT_WIDTH*zoom);j++) {
        unsigned char mask = 1 << (j/zoom);
        unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

        drawPixel(x+j, y+i, col);
    }
    glyph += (i%zoom) ? 0 : FONT_BPL;
    }
}

void micrOS_WriteLine(int x, int y, char *s, unsigned char attr, int zoom) {
    while (*s) {
       if (*s == '\r') {
              x = 0;
           } else if(*s == '\n') {
              x = 0; y += (FONT_HEIGHT*zoom);
           } else {
          drawChar(*s, x, y, attr, zoom);
              x += (FONT_WIDTH*zoom);
           }
        s++;
    }
}

int strlen(const char *str) {
    const char *s;

    for (s = str; *s; ++s);
    return (s - str);
}

void microPrint(char *string) {
    if (current_X + (strlen(string) * 8)  >= 1920) {
       current_X = 0; current_Y += 8;
    }
    if (current_Y + 8 >= 1080) {
       current_Y = 0;
    }
    micrOS_WriteLine(current_X, current_Y, string, 0x0f, 1);
    current_X += (strlen(string) * 8);

}

void microPrint_NewLine(void) {
    current_X = 0; current_Y += 8;
}

void microPrint_Character(unsigned char b) {
    unsigned int n;
    int c;
    for(c=4;c>=0;c-=4) {
        n=(b>>c)&0xF;
        n+=n>9?0x37:0x30;
        microPrint((char *)&n);
    }
    microPrint(" ");
}

void microPrint_Hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        n=(d>>c)&0xF;
        n+=n>9?0x37:0x30;
        microPrint((char *)&n);
    }
    microPrint(" ");
}
