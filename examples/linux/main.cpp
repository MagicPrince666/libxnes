/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xnes.h>
#include <memory>
#if defined(FRAME_BUFFER)
#include "framebuffer.h"
#else
#include "sdlshow.h"
#endif

#if defined(__unix__) || defined(__APPLE__)
#ifdef __GLIBC__
#include <execinfo.h>
#endif
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <iostream>

#define PRINT_SIZE_ 100
const char *g_exe_name;

static void _signal_handler(int signum)
{
#ifdef __GLIBC__
    void *array[PRINT_SIZE_];
    char **strings;

    size_t size = backtrace(array, PRINT_SIZE_);
    strings     = backtrace_symbols(array, size);

    if (strings == nullptr) {
        fprintf(stderr, "backtrace_symbols");
        exit(EXIT_FAILURE);
    }
#endif
    switch (signum) {
    case SIGSEGV:
        fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");
        break;

    case SIGPIPE:
        fprintf(stderr, "widebright received SIGPIPE! Stack trace:\n");
        break;

    case SIGFPE:
        fprintf(stderr, "widebright received SIGFPE! Stack trace:\n");
        break;

    case SIGABRT:
        fprintf(stderr, "widebright received SIGABRT! Stack trace:\n");
        break;

    default:
        break;
    }
#ifdef __GLIBC__
    std::string path = std::string(g_exe_name) + ".log";
    std::ofstream outfile(path, std::ios::out | std::ios::app);
    if (outfile.is_open()) {
        outfile << "Commit ID: " << GIT_VERSION << std::endl;
        outfile << "Git path: " << GIT_PATH << std::endl;
        outfile << "Compile time: " << __TIME__ << " " << __DATE__ << std::endl;
    }
    for (size_t i = 0; i < size; i++) {
        fprintf(stderr, "%ld %s \n", i, strings[i]);
        if (outfile.is_open()) {
            outfile << strings[i] << std::endl;
        }
    }
    if (outfile.is_open()) {
        outfile.close();
    }
    free(strings);
#endif
    signal(signum, SIG_DFL); /* 还原默认的信号处理handler */
    fprintf(stderr, "%s quit execute now\n", g_exe_name);
    fflush(stderr);
    exit(-1);
}
#endif

static void print_version(void)
{
    std::cout << "==============================================================" << std::endl;
    std::cout << "This is libnes!!" << std::endl;
    std::cout << "commit ID: " << GIT_VERSION << std::endl;
    std::cout << "git path: " << GIT_PATH << std::endl;
    std::cout << "Compile time: " << __TIME__ << " : " << __DATE__ << std::endl;
    std::cout << "Author:          Leo Huang" << std::endl;
    std::cout << "==============================================================" << std::endl;
}

int main(int argc, char * argv[])
{
#if defined(__unix__) || defined(__APPLE__)
    signal(SIGPIPE, _signal_handler); // SIGPIPE，管道破裂。
    signal(SIGSEGV, _signal_handler); // SIGSEGV，非法内存访问
    signal(SIGFPE, _signal_handler);  // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
    signal(SIGABRT, _signal_handler); // SIGABRT，由调用abort函数产生，进程非正常退出
    g_exe_name = argv[0];
#endif

    print_version();

#if defined(FRAME_BUFFER)
	std::unique_ptr<FrameBuffer> fb(new FrameBuffer(0));
	fb->Init();
#else
	std::unique_ptr<SdlShow> sdl(new SdlShow());
	sdl->SdlLoop(argc, argv);
#endif
	return 0;
}
