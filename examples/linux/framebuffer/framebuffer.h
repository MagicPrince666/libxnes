/**
 * @file framebuffer.h
 * @author 黄李全 (846863428@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-23
 * @copyright 个人版权所有 Copyright (c) 2023
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

#define BLACK   0x000000      /* Black */
#define RED     0xDC143C      /* Red */
#define GREEN   0x008000      /* Green */
#define YELLOW  0xFFFFE0      /* Yellow */
#define BLUE    0x0000FF      /* Blue */
#define WHITE   0xFFFFFF      /* White */

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
