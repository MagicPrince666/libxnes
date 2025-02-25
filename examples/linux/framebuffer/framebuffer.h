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

#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>

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

#define RGB_BLACK 0x000000
#define RGB_WHITE 0xFFFFFF
#define RGB_GRAY 0x808080
#define RGB_RED 0xFF0000
#define RGB_ORANGE 0xFFA500
#define RGB_GREEN 0x008000
#define RGB_BLUE 0x0000FF
#define RGB_SILVER 0xC0C0C0
#define RGB_BROWN 0xA52A2A
#define RGB_CYAN 0x00FFFF
#define RGB_CERISE 0xDE3163
#define RGB_YELLOW 0xFFFF00
#define RGB_GOLDEN 0xFFD700
#define RGB_VERMILION 0xFF4D00

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
    /**
     * @brief 将值打印到屏幕
     * @param x
     * @param y
     * @param value
     * @param maxlen
     * @param color
     * @param clear
     * @param clearlen
     * @return int
     */
    int32_t PutValue(int32_t x, int32_t y, int32_t value, uint32_t maxlen,
                     uint32_t color, bool clear, int32_t clearlen);

    /**
     * @brief 画点
     * @param x
     * @param y
     * @param color
     */
    void DrawPixel(int32_t x, int32_t y, uint32_t color);

    /**
     * @brief 画线
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     */
    void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);

    /**
     * @brief 画圆
     * @param x
     * @param y
     * @param r
     * @param color
     */
    void DrawCircle(int32_t x, int32_t y, int32_t r, uint32_t color);

    /**
     * @brief 清空区域
     * @param x
     * @param y
     * @param w
     * @param h
     */
    void ClearArea(int32_t x, int32_t y, int32_t w, int32_t h);

    /**
     * @brief 画矩形
     * (x1,y1),(x2,y2):矩形的对角坐标
     */
    void DrawRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);

private:
    struct fb_info {
        int fd;

        void *ptr;
        struct fb_var_screeninfo var;
        struct fb_fix_screeninfo fix;

        uint32_t bytespp;
    };
    struct fb_info *fb_info_;
    int fb_num_;

    void PutChar(int x, int y, char c, uint32_t color);
};

#endif
