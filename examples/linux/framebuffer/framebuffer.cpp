#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include "font_8x8.h"
#include "framebuffer.h"

FrameBuffer::FrameBuffer(int fb_num)
    : fb_num_(fb_num)
{
}

FrameBuffer::~FrameBuffer()
{
    if (fb_info_->fd) {
        munmap(fb_info_->ptr, fb_info_->fix.smem_len);
        close(fb_info_->fd);
    }
    delete fb_info_;
}

bool FrameBuffer::Init()
{
    fb_info_ = new fb_info;
    // int tty = open("/dev/tty1", O_RDWR);

    // if(ioctl(tty, KDSETMODE, KD_GRAPHICS) == -1)
    // 	printf("Failed to set graphics mode on tty1\n");

    std::string dev = "/dev/fb" + std::to_string(fb_num_);
    std::cout << "device = " << dev << std::endl;
    int fd = open(dev.c_str(), O_RDWR);

    ASSERT(fd >= 0);

    fb_info_->fd = fd;
    int stat     = ioctl(fd, FBIOGET_FSCREENINFO, &fb_info_->fix);
    if (stat < 0) {
        perror("Error getting fix screeninfo");
        return false;
    }
    stat = ioctl(fd, FBIOGET_VSCREENINFO, &fb_info_->var);
    if (stat < 0) {
        perror("Error getting var screeninfo");
        return false;
    }
    // stat = ioctl(fd, FBIOPUT_VSCREENINFO, &fb_info_->var);
    // if (stat < 0) {
    //     perror("Error setting mode");
    //     return false;
    // }

    printf("fb res %dx%d virtual %dx%d, smem_len %d, bpp %d\n",
           fb_info_->var.xres, fb_info_->var.yres,
           fb_info_->var.xres_virtual, fb_info_->var.yres_virtual,
           fb_info_->fix.smem_len, fb_info_->var.bits_per_pixel);

    /*计算屏幕缓冲区大小*/
    int32_t screensize = fb_info_->var.xres * fb_info_->var.yres * fb_info_->var.bits_per_pixel / 8;

    fb_info_->ptr = (uint8_t *)mmap(nullptr, screensize, PROT_WRITE | PROT_READ,
                                    MAP_SHARED, fb_info_->fd, 0);

    ASSERT(fb_info_->ptr != MAP_FAILED);

    ScreenSolid(RGB_BLUE);
    return true;
}

void FrameBuffer::ClearArea(int x, int y, int w, int h)
{
    int i = 0;
    int loc;
    char *fbuffer                 = (char *)fb_info_->ptr;
    struct fb_var_screeninfo *var = &fb_info_->var;
    struct fb_fix_screeninfo *fix = &fb_info_->fix;

    for (i = 0; i < h; i++) {
        loc = (x + var->xoffset) * (var->bits_per_pixel / 8) + (y + i + var->yoffset) * fix->line_length;
        memset(fbuffer + loc, 0, w * var->bits_per_pixel / 8);
    }
}

void FrameBuffer::PutChar(int x, int y, char c,
                          unsigned color)
{
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
}

void FrameBuffer::ScreenSolid(uint32_t color)
{
    uint32_t x, y;
    uint32_t h = fb_info_->var.yres;
    uint32_t w = fb_info_->var.xres;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            DrawPixel(x, y, color);
        }
    }
}

int32_t FrameBuffer::PutValue(int32_t x, int32_t y, int32_t value, uint32_t maxlen,
                              uint32_t color, bool clear, int32_t clearlen)
{
    int32_t w    = 0;
    char str[40] = {0};

    int32_t len = snprintf(str, sizeof(str), "%d", value);
    str[len]    = 0;

    if (clear) {
        ClearArea(x, y, clearlen * 8, 8);
    }

    for (uint32_t i = 0; i < strlen(str) && i < maxlen; i++) {
        PutChar((x + 8 * i), y, str[i], color);
        w += 8;
    }

    return w;
}

void FrameBuffer::DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    int32_t t;
    int32_t xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int32_t incx, incy, uRow, uCol;
    delta_x = x2 - x1; // 计算坐标增量
    delta_y = y2 - y1;
    uRow    = x1;
    uCol    = y1;
    if (delta_x > 0) {
        incx = 1; // 设置单步方向
    } else if (delta_x == 0) {
        incx = 0; // 垂直线
    } else {
        incx    = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0; // 水平线
    } else {
        incy    = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y) {
        distance = delta_x; // 选取基本增量坐标轴
    } else {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++) { // 画线输出
        DrawPixel(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void FrameBuffer::DrawCircle(int32_t x, int32_t y, int32_t r, uint32_t color)
{
    int32_t a, b, num;
    a = 0;
    b = r;
    while (2 * b * b >= r * r) {        // 1/8圆即可
        DrawPixel(x + a, y - b, color); // 0~1
        DrawPixel(x - a, y - b, color); // 0~7
        DrawPixel(x - a, y + b, color); // 4~5
        DrawPixel(x + a, y + b, color); // 4~3

        DrawPixel(x + b, y + a, color); // 2~3
        DrawPixel(x + b, y - a, color); // 2~1
        DrawPixel(x - b, y - a, color); // 6~7
        DrawPixel(x - b, y + a, color); // 6~5

        a++;
        num = (a * a + b * b) - r * r;
        if (num > 0) {
            b--;
            a--;
        }
    }
}

void FrameBuffer::DrawRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    DrawLine(x1, y1, x2, y1, color);
    DrawLine(x1, y1, x1, y2, color);
    DrawLine(x1, y2, x2, y2, color);
    DrawLine(x2, y1, x2, y2, color);
}
