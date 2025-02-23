#ifndef __SDL_SHOW_H__
#define __SDL_SHOW_H__

struct window_context_t * window_context_alloc(void);
void window_context_reload(struct window_context_t * wctx, const char * filename);
void SdlLoop(int argc, char * argv[]);

#endif
