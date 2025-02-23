#include "sdlshow.h"
#include <xnes.h>

SdlShow::SdlShow() {}

SdlShow::~SdlShow() {}

uint64_t SdlShow::KtimeGet(void)
{
    return SDL_GetPerformanceCounter() * 1000000000.0 / SDL_GetPerformanceFrequency();
}

void SdlShow::WindowAudioCallback(void *ctx, float v)
{
    struct window_context_t *wctx = (struct window_context_t *)ctx;
    SDL_QueueAudio(wctx->audio, &v, sizeof(float));
    // SDL_QueueAudio(wctx->audio, &v, sizeof(float));
}

struct window_context_t *SdlShow::WindowContextAlloc(void)
{
    struct window_context_t *wctx;
    Uint32 r, g, b, a;
    int bpp;

    wctx = (struct window_context_t *)malloc(sizeof(struct window_context_t));
    if (!wctx)
        return NULL;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
    SDL_DisableScreenSaver();
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    wctx->window = SDL_CreateWindow("XNES - The Nintendo Entertainment System Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * SDL_SCREEN_SCALE, 240 * SDL_SCREEN_SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
    wctx->screen = SDL_GetWindowSurface(wctx->window);
    SDL_GetWindowSize(wctx->window, &wctx->width, &wctx->height);

    SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bpp, &r, &g, &b, &a);
    wctx->surface  = SDL_CreateRGBSurface(SDL_SWSURFACE, 256 * SDL_SCREEN_SCALE, 240 * SDL_SCREEN_SCALE, bpp, r, g, b, a);
    wctx->renderer = SDL_CreateSoftwareRenderer(wctx->surface);
    wctx->joy[0]   = NULL;
    wctx->joy[1]   = NULL;

    wctx->wantspec.freq     = 48000;
    wctx->wantspec.format   = AUDIO_F32SYS;
    wctx->wantspec.channels = 2;
    wctx->wantspec.samples  = 2048;
    wctx->wantspec.callback = NULL;
    wctx->audio             = SDL_OpenAudioDevice(0, 0, &wctx->wantspec, &wctx->havespec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    SDL_ClearQueuedAudio(wctx->audio);
    SDL_PauseAudioDevice(wctx->audio, 0);

    wctx->nes       = NULL;
    wctx->state     = NULL;
    wctx->rewind    = 0;
    wctx->timestamp = KtimeGet();
    wctx->elapsed   = 0;

    return wctx;
}

void SdlShow::WindowContextFree(struct window_context_t *wctx)
{
    if (wctx) {
        if (wctx->nes)
            xnes_ctx_free(wctx->nes);
        if (wctx->state)
            xnes_state_free(wctx->state);
        if (wctx->joy[0]) {
            SDL_JoystickClose(wctx->joy[0]);
            wctx->joy[0] = NULL;
        }
        if (wctx->joy[1]) {
            SDL_JoystickClose(wctx->joy[1]);
            wctx->joy[1] = NULL;
        }
        SDL_CloseAudio();
        if (wctx->screen)
            SDL_FreeSurface(wctx->screen);
        if (wctx->surface)
            SDL_FreeSurface(wctx->surface);
        if (wctx->renderer)
            SDL_DestroyRenderer(wctx->renderer);
        if (wctx->window)
            SDL_DestroyWindow(wctx->window);
        free(wctx);
    }
}

void SdlShow::WindowContextScreenRefresh(struct window_context_t *wctx)
{
    uint32_t *fb = (uint32_t *)(wctx->surface->pixels);
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            uint32_t c = xnes_get_pixel(wctx->nes, x, y);
            int p      = ((y * SDL_SCREEN_SCALE) * (256 * SDL_SCREEN_SCALE)) + x * SDL_SCREEN_SCALE;
            for (int j = 0; j < SDL_SCREEN_SCALE; j++) {
                for (int i = 0; i < SDL_SCREEN_SCALE; i++)
                    fb[p + i] = c;
                p += (256 * SDL_SCREEN_SCALE);
            }
        }
    }
    SDL_BlitSurface(wctx->surface, NULL, wctx->screen, NULL);
    SDL_UpdateWindowSurface(wctx->window);
}

void SdlShow::WindowContextUpdate(struct window_context_t *wctx)
{
    if (wctx->nes) {
        if (wctx->rewind) {
            if ((KtimeGet() - wctx->timestamp) >= wctx->elapsed) {
                wctx->timestamp = KtimeGet();
                wctx->elapsed   = 16666666;
                xnes_state_pop(wctx->state);
                WindowContextScreenRefresh(wctx);
            } else
                SDL_Delay(1);
        } else {
            if ((KtimeGet() - wctx->timestamp) >= wctx->elapsed) {
                wctx->timestamp = KtimeGet();
                wctx->elapsed   = xnes_step_frame(wctx->nes);
                WindowContextScreenRefresh(wctx);
                xnes_state_push(wctx->state);
            } else
                SDL_Delay(1);
        }
    } else
        SDL_Delay(1);
}

void *SdlShow::FileLoad(const char *filename, uint64_t *len)
{
    FILE *in = fopen(filename, "rb");
    if (in) {
        uint64_t offset = 0, bufsize = 8192;
        char *buf = (char *)malloc(bufsize);
        while (1) {
            uint64_t len = bufsize - offset;
            uint64_t n   = fread(buf + offset, 1, len, in);
            offset += n;
            if (n < len)
                break;
            bufsize *= 2;
            buf = (char *)realloc(buf, bufsize);
        }
        if (len)
            *len = offset;
        fclose(in);
        return buf;
    }
    return NULL;
}

void SdlShow::WindowContextReload(struct window_context_t *wctx, const char *filename)
{
    void *buf;
    uint64_t len;

    if (wctx) {
        if (wctx->nes) {
            xnes_ctx_free(wctx->nes);
            wctx->nes = NULL;
        }
        buf = FileLoad(filename, &len);
        if (buf) {
            wctx->nes = xnes_ctx_alloc(buf, len);
            if (wctx->nes) {
                if (wctx->state)
                    xnes_state_free(wctx->state);
                wctx->state = xnes_state_alloc(wctx->nes, 60 * 30);

                SDL_ClearQueuedAudio(wctx->audio);
                xnes_set_audio(wctx->nes, wctx, SdlShow::WindowAudioCallback, wctx->havespec.freq);
            }
            free(buf);
        }
    }
}

void SdlShow::SdlLoop(int argc, char *argv[])
{
    struct window_context_t *wctx = WindowContextAlloc();
    SDL_Event e;
    int done = 0;

    if (argc > 1) {
        WindowContextReload(wctx, argv[1]);
    }
    while (!done) {
        if (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                done = 1;
                break;

            case SDL_DROPFILE:
                WindowContextReload(wctx, e.drop.file);
                SDL_free(e.drop.file);
                break;

            case SDL_JOYDEVICEADDED:
                for (int i = 0; i < SDL_NumJoysticks(); i++) {
                    if (!(wctx->joy[0] && SDL_JoystickGetAttached(wctx->joy[0])) || !(wctx->joy[1] && SDL_JoystickGetAttached(wctx->joy[1]))) {
                        SDL_Joystick *joy = SDL_JoystickOpen(i);
                        if (joy) {
                            if (!(wctx->joy[0] && SDL_JoystickGetAttached(wctx->joy[0])))
                                wctx->joy[0] = joy;
                            else if (!(wctx->joy[1] && SDL_JoystickGetAttached(wctx->joy[1])))
                                wctx->joy[1] = joy;
                        }
                    } else {
                        break;
                    }
                }
                break;

            case SDL_JOYDEVICEREMOVED:
                if (wctx->joy[0] && !SDL_JoystickGetAttached(wctx->joy[0])) {
                    SDL_JoystickClose(wctx->joy[0]);
                    wctx->joy[0] = NULL;
                }
                if (wctx->joy[1] && !SDL_JoystickGetAttached(wctx->joy[1])) {
                    SDL_JoystickClose(wctx->joy[1]);
                    wctx->joy[1] = NULL;
                }
                break;

            default:
                break;
            }
            if (wctx->nes) {
                switch (e.type) {
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT)
                        xnes_controller_zapper(&wctx->nes->ctl, e.button.x / SDL_SCREEN_SCALE, e.button.y / SDL_SCREEN_SCALE, 1);
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_LEFT)
                        xnes_controller_zapper(&wctx->nes->ctl, e.button.x / SDL_SCREEN_SCALE, e.button.y / SDL_SCREEN_SCALE, 0);
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        xnes_reset(wctx->nes);
                        break;

                    case SDLK_F1:
                        xnes_set_speed(wctx->nes, 0.5);
                        break;

                    case SDLK_F2:
                        xnes_set_speed(wctx->nes, 2.0);
                        break;

                    case SDLK_r:
                        wctx->rewind = 1;
                        break;

                    case SDLK_w:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
                        break;
                    case SDLK_s:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
                        break;
                    case SDLK_a:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
                        break;
                    case SDLK_d:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
                        break;
                    case SDLK_k:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                        break;
                    case SDLK_j:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                        break;
                    case SDLK_SPACE:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
                        break;
                    case SDLK_RETURN:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
                        break;

                    case SDLK_UP:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
                        break;
                    case SDLK_DOWN:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
                        break;
                    case SDLK_LEFT:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
                        break;
                    case SDLK_RIGHT:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
                        break;
                    case SDLK_KP_6:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                        break;
                    case SDLK_KP_5:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                        break;
                    case SDLK_KP_8:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
                        break;
                    case SDLK_KP_9:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
                        break;

                    default:
                        break;
                    }
                    break;

                case SDL_KEYUP:
                    switch (e.key.keysym.sym) {
                    case SDLK_F1:
                        xnes_set_speed(wctx->nes, 1.0);
                        break;

                    case SDLK_F2:
                        xnes_set_speed(wctx->nes, 1.0);
                        break;

                    case SDLK_r:
                        wctx->rewind = 0;
                        break;

                    case SDLK_w:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP);
                        break;
                    case SDLK_s:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_DOWN);
                        break;
                    case SDLK_a:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT);
                        break;
                    case SDLK_d:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_RIGHT);
                        break;
                    case SDLK_k:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                        break;
                    case SDLK_j:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                        break;
                    case SDLK_SPACE:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
                        break;
                    case SDLK_RETURN:
                        xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
                        break;

                    case SDLK_UP:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP);
                        break;
                    case SDLK_DOWN:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_DOWN);
                        break;
                    case SDLK_LEFT:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT);
                        break;
                    case SDLK_RIGHT:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_RIGHT);
                        break;
                    case SDLK_KP_6:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                        break;
                    case SDLK_KP_5:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                        break;
                    case SDLK_KP_8:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
                        break;
                    case SDLK_KP_9:
                        xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
                        break;

                    default:
                        break;
                    }
                    break;

                case SDL_JOYAXISMOTION:
                    if (e.jaxis.which == SDL_JoystickInstanceID(wctx->joy[0])) {
                        switch (e.jaxis.axis) {
                        case 0:
                            if (e.jaxis.value < -3200)
                                xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
                            else if (e.jaxis.value > 3200)
                                xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
                            else
                                xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT | XNES_JOYSTICK_RIGHT);
                            break;
                        case 1:
                            if (e.jaxis.value < -3200)
                                xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
                            else if (e.jaxis.value > 3200)
                                xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
                            else
                                xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP | XNES_JOYSTICK_DOWN);
                            break;
                        case 2:
                            if (e.jaxis.value < -3200)
                                xnes_set_speed(wctx->nes, 1.0);
                            else if (e.jaxis.value > 3200)
                                xnes_set_speed(wctx->nes, 0.5);
                            break;
                        case 5:
                            if (e.jaxis.value < -3200)
                                xnes_set_speed(wctx->nes, 1.0);
                            else if (e.jaxis.value > 3200)
                                xnes_set_speed(wctx->nes, 2.0);
                            break;
                        default:
                            break;
                        }
                    } else if (e.jaxis.which == SDL_JoystickInstanceID(wctx->joy[1])) {
                        switch (e.jaxis.axis) {
                        case 0:
                            if (e.jaxis.value < -3200)
                                xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
                            else if (e.jaxis.value > 3200)
                                xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
                            else
                                xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT | XNES_JOYSTICK_RIGHT);
                            break;
                        case 1:
                            if (e.jaxis.value < -3200)
                                xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
                            else if (e.jaxis.value > 3200)
                                xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
                            else
                                xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP | XNES_JOYSTICK_DOWN);
                            break;
                        case 2:
                            if (e.jaxis.value < -3200)
                                xnes_set_speed(wctx->nes, 1.0);
                            else if (e.jaxis.value > 3200)
                                xnes_set_speed(wctx->nes, 0.5);
                            break;
                        case 5:
                            if (e.jaxis.value < -3200)
                                xnes_set_speed(wctx->nes, 1.0);
                            else if (e.jaxis.value > 3200)
                                xnes_set_speed(wctx->nes, 2.0);
                            break;
                        default:
                            break;
                        }
                    }
                    break;

                case SDL_JOYHATMOTION:
                    if (e.jhat.which == SDL_JoystickInstanceID(wctx->joy[0])) {
                        switch (e.jhat.hat) {
                        case 0:
                            xnes_controller_joystick_p1(&wctx->nes->ctl, (e.jhat.value & 0x8) ? XNES_JOYSTICK_LEFT : 0, (e.jhat.value & 0x8) ? 0 : XNES_JOYSTICK_LEFT);
                            xnes_controller_joystick_p1(&wctx->nes->ctl, (e.jhat.value & 0x4) ? XNES_JOYSTICK_DOWN : 0, (e.jhat.value & 0x4) ? 0 : XNES_JOYSTICK_DOWN);
                            xnes_controller_joystick_p1(&wctx->nes->ctl, (e.jhat.value & 0x2) ? XNES_JOYSTICK_RIGHT : 0, (e.jhat.value & 0x2) ? 0 : XNES_JOYSTICK_RIGHT);
                            xnes_controller_joystick_p1(&wctx->nes->ctl, (e.jhat.value & 0x1) ? XNES_JOYSTICK_UP : 0, (e.jhat.value & 0x1) ? 0 : XNES_JOYSTICK_UP);
                            break;
                        default:
                            break;
                        }
                    } else if (e.jhat.which == SDL_JoystickInstanceID(wctx->joy[1])) {
                        switch (e.jhat.hat) {
                        case 0:
                            xnes_controller_joystick_p2(&wctx->nes->ctl, (e.jhat.value & 0x8) ? XNES_JOYSTICK_LEFT : 0, (e.jhat.value & 0x8) ? 0 : XNES_JOYSTICK_LEFT);
                            xnes_controller_joystick_p2(&wctx->nes->ctl, (e.jhat.value & 0x4) ? XNES_JOYSTICK_DOWN : 0, (e.jhat.value & 0x4) ? 0 : XNES_JOYSTICK_DOWN);
                            xnes_controller_joystick_p2(&wctx->nes->ctl, (e.jhat.value & 0x2) ? XNES_JOYSTICK_RIGHT : 0, (e.jhat.value & 0x2) ? 0 : XNES_JOYSTICK_RIGHT);
                            xnes_controller_joystick_p2(&wctx->nes->ctl, (e.jhat.value & 0x1) ? XNES_JOYSTICK_UP : 0, (e.jhat.value & 0x1) ? 0 : XNES_JOYSTICK_UP);
                            break;
                        default:
                            break;
                        }
                    }
                    break;

                case SDL_JOYBUTTONDOWN:
                    if (e.jbutton.which == SDL_JoystickInstanceID(wctx->joy[0])) {
                        switch (e.jbutton.button) {
                        case 0: /* B */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                            break;
                        case 1: /* A */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                            break;
                        case 2: /* Y */
                            xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                            break;
                        case 3: /* X */
                            xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                            break;
                        case 4: /* L */
                            wctx->rewind = 1;
                            break;
                        case 5: /* R */
                            xnes_set_speed(wctx->nes, 0.5);
                            break;
                        case 6: /* Select */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
                            break;
                        case 7: /* Start */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
                            break;
                        default:
                            break;
                        }
                    } else if (e.jbutton.which == SDL_JoystickInstanceID(wctx->joy[1])) {
                        switch (e.jbutton.button) {
                        case 0: /* B */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                            break;
                        case 1: /* A */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                            break;
                        case 2: /* Y */
                            xnes_controller_joystick_p2_turbo(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
                            break;
                        case 3: /* X */
                            xnes_controller_joystick_p2_turbo(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
                            break;
                        case 4: /* L */
                            wctx->rewind = 1;
                            break;
                        case 5: /* R */
                            xnes_set_speed(wctx->nes, 0.5);
                            break;
                        case 6: /* Select */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
                            break;
                        case 7: /* Start */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
                            break;
                        default:
                            break;
                        }
                    }
                    break;

                case SDL_JOYBUTTONUP:
                    if (e.jbutton.which == SDL_JoystickInstanceID(wctx->joy[0])) {
                        switch (e.jbutton.button) {
                        case 0: /* B */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                            break;
                        case 1: /* A */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                            break;
                        case 2: /* Y */
                            xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                            break;
                        case 3: /* X */
                            xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                            break;
                        case 4: /* L */
                            wctx->rewind = 0;
                            break;
                        case 5: /* R */
                            xnes_set_speed(wctx->nes, 1.0);
                            break;
                        case 6: /* Select */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
                            break;
                        case 7: /* Start */
                            xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
                            break;
                        default:
                            break;
                        }
                    } else if (e.jbutton.which == SDL_JoystickInstanceID(wctx->joy[1])) {
                        switch (e.jbutton.button) {
                        case 0: /* B */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                            break;
                        case 1: /* A */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                            break;
                        case 2: /* Y */
                            xnes_controller_joystick_p2_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
                            break;
                        case 3: /* X */
                            xnes_controller_joystick_p2_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
                            break;
                        case 4: /* L */
                            wctx->rewind = 0;
                            break;
                        case 5: /* R */
                            xnes_set_speed(wctx->nes, 1.0);
                            break;
                        case 6: /* Select */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
                            break;
                        case 7: /* Start */
                            xnes_controller_joystick_p2(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
                            break;
                        default:
                            break;
                        }
                    }
                    break;

                default:
                    break;
                }
            }
        }
        WindowContextUpdate(wctx);
    }
    WindowContextFree(wctx);
    SDL_Quit();
}