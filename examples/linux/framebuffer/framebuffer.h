/*
 * common.h
 *
 * Author: Tomi Valkeinen <tomi.valkeinen@nokia.com>
 * Copyright (C) 2009-2012 Tomi Valkeinen

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <stdint.h>
#include <stdio.h>
#if defined(__linux__)
#include <linux/fb.h>
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define ASSERT(x)                                                \
    if (!(x)) {                                                  \
        perror("assert(" __FILE__ ":" TOSTRING(__LINE__) "): "); \
        exit(1);                                                 \
    }
#define FBCTL0(ctl)                                              \
    if (ioctl(fd, ctl)) {                                        \
        perror("fbctl0(" __FILE__ ":" TOSTRING(__LINE__) "): "); \
        exit(1);                                                 \
    }
#define FBCTL1(ctl, arg1)                                        \
    if (ioctl(fd, ctl, arg1)) {                                  \
        perror("fbctl1(" __FILE__ ":" TOSTRING(__LINE__) "): "); \
        exit(1);                                                 \
    }

#define IOCTL0(fd, ctl)                                          \
    if (ioctl(fd, ctl)) {                                        \
        perror("ioctl0(" __FILE__ ":" TOSTRING(__LINE__) "): "); \
        exit(1);                                                 \
    }
#define IOCTL1(fd, ctl, arg1)                                    \
    if (ioctl(fd, ctl, arg1)) {                                  \
        perror("ioctl1(" __FILE__ ":" TOSTRING(__LINE__) "): "); \
        exit(1);                                                 \
    }

struct fb_info {
    int fd;

    void *ptr;
#if defined(__linux__)
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
#endif
    uint32_t bytespp;
};

class FrameBuffer
{
public:
    FrameBuffer(int fb_num);
    ~FrameBuffer();

    /**
     * @brief 初始化framebuffer
     * @return true 
     * @return false 
     */
    bool Init();

    /**
     * @brief 刷屏
     * @param color 
     */
    void ScreenSolid(uint32_t color);

    /**
     * @brief 绘制字符
     * @param x 
     * @param y 
     * @param s 
     * @param maxlen 
     * @param color 
     * @param clear 
     * @param clearlen 
     * @return int 
     */
    int PutString(int x, int y, char *s, int maxlen,
                  int color, bool clear, int clearlen);

private:
    void DrawPixel(int x, int y, uint32_t color);
    void ClearArea(int x, int y, int w, int h);
    void PutChar(int x, int y, char c, uint32_t color);

    struct fb_info *fb_info_;
    int fb_num_;
};

#endif
