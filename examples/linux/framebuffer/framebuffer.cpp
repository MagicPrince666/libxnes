#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#if defined(__linux__)
#include <linux/fb.h>
#include <linux/kd.h>
#endif

#include "framebuffer.h"
#include "font_8x8.h"

FrameBuffer::FrameBuffer(int fb_num)
:fb_num_(fb_num)
{
}

FrameBuffer::~FrameBuffer()
{
    if (fb_info_->fd) {
        close(fb_info_->fd);
    }
    delete fb_info_;
}

bool FrameBuffer::Init()
{
    #if defined(__linux__)
    char str[64];
    int fd   = -1;
    fb_info_ = new fb_info;
    // int tty = open("/dev/tty1", O_RDWR);

    // if(ioctl(tty, KDSETMODE, KD_GRAPHICS) == -1)
    // 	printf("Failed to set graphics mode on tty1\n");

    sprintf(str, "/dev/fb%d", fb_num_);
    fd = open(str, O_RDWR);

    ASSERT(fd >= 0);

    fb_info_->fd = fd;
    IOCTL1(fd, FBIOGET_VSCREENINFO, &fb_info_->var);
    IOCTL1(fd, FBIOGET_FSCREENINFO, &fb_info_->fix);

    printf("fb res %dx%d virtual %dx%d, line_len %d, bpp %d\n",
           fb_info_->var.xres, fb_info_->var.yres,
           fb_info_->var.xres_virtual, fb_info_->var.yres_virtual,
           fb_info_->fix.line_length, fb_info_->var.bits_per_pixel);

    void *ptr = mmap(0,
                     fb_info_->var.yres_virtual * fb_info_->fix.line_length,
                     PROT_WRITE | PROT_READ,
                     MAP_SHARED, fd, 0);

    ASSERT(ptr != MAP_FAILED);

    fb_info_->ptr = ptr;
#endif
    return true;
}

void FrameBuffer::ClearArea(int x, int y, int w, int h)
{
#if defined(__linux__)
    int i = 0;
    int loc;
    char *fbuffer                 = (char *)fb_info_->ptr;
    struct fb_var_screeninfo *var = &fb_info_->var;
    struct fb_fix_screeninfo *fix = &fb_info_->fix;

    for (i = 0; i < h; i++) {
        loc = (x + var->xoffset) * (var->bits_per_pixel / 8) + (y + i + var->yoffset) * fix->line_length;
        memset(fbuffer + loc, 0, w * var->bits_per_pixel / 8);
    }
#endif
}

void FrameBuffer::PutChar(int x, int y, char c,
                              unsigned color)
{
#if defined(__linux__)
    int i, j, bits, loc;
    uint8_t *p8;
    uint16_t *p16;
    uint32_t *p32;
    struct fb_var_screeninfo *var = &fb_info_->var;
    struct fb_fix_screeninfo *fix = &fb_info_->fix;

    for (i = 0; i < 8; i++) {
        bits = fontdata_8x8[8 * c + i];
        for (j = 0; j < 8; j++) {
            loc = (x + j + var->xoffset) * (var->bits_per_pixel / 8) + (y + i + var->yoffset) * fix->line_length;
            if (loc >= 0 && loc < fix->smem_len &&
                ((bits >> (7 - j)) & 1)) {
                switch (var->bits_per_pixel) {
                case 8:
                    p8  = (uint8_t *)(fb_info_->ptr) + loc;
                    *p8 = color;
                case 16:
                    p16  = (uint16_t *)(fb_info_->ptr) + loc;
                    *p16 = color;
                    break;
                case 24:
                case 32:
                    p32  = (uint32_t *)(fb_info_->ptr) + loc;
                    *p32 = color;
                    break;
                }
            }
        }
    }
#endif
}

int FrameBuffer::PutString(int x, int y, char *s, int maxlen,
                           int color, bool clear, int clearlen)
{
    int i;
    int w = 0;

    if (clear) {
        ClearArea(x, y, clearlen * 8, 8);
    }

    for (i = 0; i < strlen(s) && i < maxlen; i++) {
        PutChar((x + 8 * i), y, s[i], color);
        w += 8;
    }

    return w;
}

void FrameBuffer::DrawPixel(int x, int y, unsigned color)
{
#if defined(__linux__)
    void *fbmem;

    fbmem = fb_info_->ptr;
    switch (fb_info_->var.bits_per_pixel) {
    case 8: {
        uint8_t *p;
        p = (uint8_t *)fbmem + fb_info_->fix.line_length * y;
        p += x;
        *p = color;
    } break;
    case 16: {
        unsigned short c;
        unsigned r = (color >> 16) & 0xff;
        unsigned g = (color >> 8) & 0xff;
        unsigned b = (color >> 0) & 0xff;
        uint16_t *p;
        r = r * 32 / 256;
        g = g * 64 / 256;
        b = b * 32 / 256;
        c = (r << 11) | (g << 5) | (b << 0);
        p = (uint16_t *)fbmem + fb_info_->fix.line_length * y;
        p += x;
        *p = c;
    } break;
    case 24: {
        unsigned char *p;
        p    = (unsigned char *)fbmem + fb_info_->fix.line_length * y + 3 * x;
        *p++ = color;
        *p++ = color >> 8;
        *p   = color >> 16;
    } break;
    default: {
        uint32_t *p;
        p = (uint32_t *)fbmem + fb_info_->fix.line_length * y;
        p += x;
        *p = color;
    } break;
    }
#endif
}

void FrameBuffer::ScreenSolid(uint32_t color)
{
#if defined(__linux__)
    uint32_t x, y;
    uint32_t h = fb_info_->var.yres;
    uint32_t w = fb_info_->var.xres;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            DrawPixel(x, y, color);
        }
    }
#endif
}
