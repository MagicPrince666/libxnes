#ifndef __SDL_SHOW_H__
#define __SDL_SHOW_H__

#include <SDL.h>
#define SDL_SCREEN_SCALE	(4)

struct window_context_t {
    SDL_Window * window;
    SDL_Surface * screen;
    SDL_Surface * surface;
    SDL_Renderer * renderer;
    SDL_Joystick * joy[2];
    SDL_AudioDeviceID audio;
    SDL_AudioSpec wantspec, havespec;
    int width;
    int height;

    struct xnes_ctx_t * nes;
    struct xnes_state_t * state;
    int rewind;
    uint64_t timestamp;
    uint64_t elapsed;
};

class SdlShow
{
public:
    SdlShow();
    ~SdlShow();

    void SdlLoop(int argc, char * argv[]);

private:
    struct window_context_t * WindowContextAlloc(void);
    void WindowContextReload(struct window_context_t * wctx, const char * filename);
    void WindowContextUpdate(struct window_context_t * wctx);
    void WindowContextScreenRefresh(struct window_context_t * wctx);
    void WindowContextFree(struct window_context_t * wctx);
    void * FileLoad(const char * filename, uint64_t * len);
    uint64_t KtimeGet(void);
    static void WindowAudioCallback(void *ctx, float v);
};

#endif
