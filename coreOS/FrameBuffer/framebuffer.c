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
#include "framebuffer.h"
#include "../GPU/mailbox.h"
#include "textmode.h"
#include "micrOS_default_wallpaper.h"
#include "../Common/coreCommon.h"

unsigned int width, height, pitch, RGBMode;
unsigned char *frameBufferAddress = 0;
void microPrint_NewLine(void);
int current_Y = 0;
int current_X = 0;

/* PC Screen Font as used by Linux Console */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int numglyph;
    unsigned int bytesperglyph;
    unsigned int height;
    unsigned int width;
    unsigned char glyphs;
} __attribute__((packed)) psf_t;
extern volatile unsigned char _binary_font_psf_start;

typedef struct {
    unsigned char  magic[4];
    unsigned int   size;
    unsigned char  type;
    unsigned char  features;
    unsigned char  width;
    unsigned char  height;
    unsigned char  baseline;
    unsigned char  underline;
    unsigned short fragments_offs;
    unsigned int   characters_offs;
    unsigned int   ligature_offs;
    unsigned int   kerning_offs;
    unsigned int   cmap_offs;
} __attribute__((packed)) sfn_t;
extern volatile unsigned char _binary_font_sfn_start;

int micrOS_Framebuffer_Init() {
    mailbox[0] = 35*4;
    mailbox[1] = MBOX_REQUEST;

    mailbox[2] = 0x48003;  //set phy wh
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 1920;         //FrameBufferInfo.width
    mailbox[6] = 1080;          //FrameBufferInfo.height

    mailbox[7] = 0x48004;  //set virt wh
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = 1920;        //FrameBufferInfo.virtual_width
    mailbox[11] = 1080;         //FrameBufferInfo.virtual_height

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
        RGBMode = mailbox[24];         //get the actual channel order
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


void drawChar(unsigned char ch, int x, int y, unsigned char attr, unsigned char foregroundColor, int zoom){
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=1;i<=(FONT_HEIGHT*zoom);i++) {
    for (int j=0;j<(FONT_WIDTH*zoom);j++) {
        unsigned char mask = 1 << (j/zoom);
        unsigned char col = (*glyph & mask) ? attr & 0x0f : (foregroundColor) >> 4;

        drawPixel(x+j, y+i, col);
    }
    glyph += (i%zoom) ? 0 : FONT_BPL;
    }
}

void micrOS_WriteLine(int x, int y, char *s, unsigned char attr, unsigned char foregroundColor, int zoom) {
    while (*s) {
       if (*s == '\r') {
              x = 0;
           } else if(*s == '\n') {
              x = 0; y += (FONT_HEIGHT*zoom);
           } else {
          drawChar(*s, x, y, attr, foregroundColor, zoom);
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

void micrOS_PaintRectangle(int x1, int y1, int x2, int y2, unsigned char attr, int fill){
    int y=y1;

    while (y <= y2) {
       int x=x1;
       while (x <= x2) {
      if ((x == x1 || x == x2) || (y == y1 || y == y2)) drawPixel(x, y, attr);
      else if (fill) drawPixel(x, y, (attr & 0xf0) >> 4);
          x++;
       }
       y++;
    }
}

void microPrint(char *string, bool newLineRequired) {
    if (current_X + (strlen(string) * 8)  >= 1920) {
       current_X = 0; current_Y += 12;
    }
    if (current_Y + 12 >= 1080) {
       current_Y = 0;
    }
    micrOS_WriteLine(current_X, current_Y, string, 0x0f, 0x00, 1);
    current_X += (strlen(string) * 8);
    if (newLineRequired == true) {
        microPrint_NewLine();
    }
}

void microPrint_NewLine(void) {
    current_X = 0; current_Y += 12;
}

void microPrint_Character(unsigned char b) {
    unsigned int n;
    int c;
    for(c=4;c>=0;c-=4) {
        n=(b>>c)&0xF;
        n+=n>9?0x37:0x30;
        microPrint((char *)&n, false);
    }
    microPrint(" ", false);
}

void microPrint_Hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        n=(d>>c)&0xF;
        n+=n>9?0x37:0x30;
        microPrint((char *)&n, false);
    }
    microPrint(" ", false);
}

void micrOS_PrintToScreen(int x, int y, char *s){
    // get our font
    psf_t *font = (psf_t*)&_binary_font_psf_start;
    // draw next character if it's not zero
    while(*s) {
        // get the offset of the glyph. Need to adjust this to support unicode table
        unsigned char *glyph = (unsigned char*)&_binary_font_psf_start +
         font->headersize + (*((unsigned char*)s)<font->numglyph?*s:0)*font->bytesperglyph;
        // calculate the offset on screen
        int offs = (y * pitch) + (x * 4);
        // variables
        int i,j, line,mask, bytesperline=(font->width+7)/8;
        // handle carrige return
        if(*s == '\r') {
            x = 0;
        } else
        // new line
        if(*s == '\n') {
            x = 0; y += font->height;
        } else {
            // display a character
            for(j=0;j<font->height;j++){
                // display one row
                line=offs;
                mask=1<<(font->width-1);
                for(i=0;i<font->width;i++){
                    // if bit set, we use white color, otherwise black
                    *((unsigned int*)(frameBufferAddress + line))=((int)*glyph) & mask?0xFFFFFF:0;
                    mask>>=1;
                    line+=4;
                }
                // adjust to next line
                glyph+=bytesperline;
                offs+=pitch;
            }
            x += (font->width+1);
        }
        // next character
        s++;
    }
}

void devicePaintPicture(char *picture_data, unsigned int picture_width, unsigned int picture_height){
#ifdef HEADER_PIXEL
    int x,y;
    unsigned char *frameBuffer = frameBufferAddress;
    char *data = picture_data, pixel[4];

    frameBuffer += (height-picture_height)/2*pitch + (width-picture_width)*2;
    
    for (y=0; y<picture_height; y++) {
        for (x=0; x<picture_width; x++) {
            HEADER_PIXEL(data, pixel);
            *((unsigned int*)frameBuffer)=RGBMode ? *((unsigned int *)&pixel) : (unsigned int)(pixel[0]<<16 | pixel[1]<<8 | pixel[2]);
            frameBuffer+=4;
        }
        frameBuffer+=pitch-picture_width*4;
    }
#endif
    return;
}

int setDefaultWallpaper(){
    devicePaintPicture(micrOS_default_wallpaper_data, wallpaper_width, wallpaper_height);
    return 0;
}
