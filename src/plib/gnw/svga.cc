#include "plib/gnw/svga.h"

#include "game/config.h"
#include "game/intface.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/grbuf.h"
#include "plib/gnw/mouse.h"
#include "plib/gnw/winmain.h"

#ifdef __3DS__
#include "platform/ctr/ctr_input.h"
#include "platform/ctr/ctr_gfx.h"
#endif

namespace fallout {

static int GNW95_init_mode_ex(int width, int height, int bpp);
static int GNW95_init_mode(int width, int height);

static bool createRenderer(int width, int height);
static void destroyRenderer();

// screen rect
Rect scr_size;

// 0x6ACA18
ScreenBlitFunc* scr_blit = GNW95_ShowRect;

SDL_Window* gSdlWindow = NULL;
SDL_Surface* gSdlSurface = NULL;
SDL_Renderer* gSdlRenderer = NULL;
SDL_Texture* gSdlTexture = NULL;
SDL_Surface* gSdlTextureSurface = NULL;

#ifdef __3DS__
SDL_Window* gSdlWindow2 = NULL;
SDL_Renderer* gSdlRenderer2 = NULL;

SDL_Rect sourceRect2 = {0, 0, 640, 480};
SDL_Rect destRect2 = {0, 0, 320, 240};

SDL_Surface* surface2 = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
#endif

// TODO: Remove once migration to update-render cycle is completed.
FpsLimiter sharedFpsLimiter;

// 0x4CAD08
int init_mode_320_200()
{
    return GNW95_init_mode_ex(320, 200, 8);
}

// 0x4CAD40
int init_mode_320_400()
{
    return GNW95_init_mode_ex(320, 400, 8);
}

// 0x4CAD5C
int init_mode_640_480_16()
{
    return -1;
}

// 0x4CAD64
int init_mode_640_480()
{
    return GNW95_init_mode(640, 480);
}

// 0x4CAD94
int init_mode_640_400()
{
    return GNW95_init_mode(640, 400);
}

// 0x4CADA8
int init_mode_800_600()
{
    return GNW95_init_mode(800, 600);
}

// 0x4CADBC
int init_mode_1024_768()
{
    return GNW95_init_mode(1024, 768);
}

// 0x4CADD0
int init_mode_1280_1024()
{
    return GNW95_init_mode(1280, 1024);
}

// 0x4CADE4
int init_vesa_mode(int mode, int width, int height, int half)
{
    if (half != 0) {
        return -1;
    }

    return GNW95_init_mode_ex(width, height, 8);
}

// 0x4CADF3
int get_start_mode()
{
    return -1;
}

// 0x4CADF8
void reset_mode()
{
}

// 0x4CAE1C
static int GNW95_init_mode_ex(int width, int height, int bpp)
{
    bool fullscreen = true;
    int scale = 1;

    Config resolutionConfig;
    if (config_init(&resolutionConfig)) {
        if (config_load(&resolutionConfig, "f1_res.ini", false)) {
            int screenWidth;
            if (config_get_value(&resolutionConfig, "MAIN", "SCR_WIDTH", &screenWidth)) {
                width = screenWidth;
            }

            int screenHeight;
            if (config_get_value(&resolutionConfig, "MAIN", "SCR_HEIGHT", &screenHeight)) {
                height = screenHeight;
            }

            bool windowed;
            if (configGetBool(&resolutionConfig, "MAIN", "WINDOWED", &windowed)) {
                fullscreen = !windowed;
            }

            int scaleValue;
            if (config_get_value(&resolutionConfig, "MAIN", "SCALE_2X", &scaleValue)) {
                scale = scaleValue + 1; // 0 = 1x, 1 = 2x
                // Only allow scaling if resulting game resolution is >= 640x480
                if ((width / scale) < 640 || (height / scale) < 480) {
                    scale = 1;
                } else {
                    width /= scale;
                    height /= scale;
                }
            }
        }
        config_exit(&resolutionConfig);
    }

    if (GNW95_init_window(width, height, fullscreen, scale) == -1) {
        return -1;
    }

    if (GNW95_init_DirectDraw(width, height, bpp) == -1) {
        return -1;
    }

    scr_size.ulx = 0;
    scr_size.uly = 0;
    scr_size.lrx = width - 1;
    scr_size.lry = height - 1;

    mouse_blit_trans = NULL;
    scr_blit = GNW95_ShowRect;
    mouse_blit = GNW95_ShowRect;

    return 0;
}

// 0x4CAECC
static int GNW95_init_mode(int width, int height)
{
    return GNW95_init_mode_ex(width, height, 8);
}

// 0x4CAEDC
int GNW95_init_window(int width, int height, bool fullscreen, int scale)
{
    if (gSdlWindow == NULL) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            return -1;
        }

        Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;

        if (fullscreen) {
            windowFlags |= SDL_WINDOW_FULLSCREEN;
        }

#ifdef __3DS__
        initializeDisplayRectMap();

        int numDisplays = SDL_GetNumVideoDisplays();
        if (numDisplays < 2) {
            SDL_Quit();
            return 1;
        }

        SDL_Rect displayBounds1, displayBounds2;
        SDL_GetDisplayBounds(0, &displayBounds1);
        SDL_GetDisplayBounds(1, &displayBounds2);

        gSdlWindow = SDL_CreateWindow("GFX_TOP", displayBounds1.x, displayBounds1.y, 400, 240, 0);
        gSdlWindow2 = SDL_CreateWindow("GFX_BOTTOM", displayBounds2.x, displayBounds2.y, 320, 240, 0);
#else
        gSdlWindow = SDL_CreateWindow(GNW95_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * scale, height * scale, windowFlags);
#endif
        if (gSdlWindow == NULL) {
#ifdef __3DS__
            GNWSystemError("SDL_CreateWindow failed\n");
#endif
            return -1;
        }

        if (!createRenderer(width, height)) {
            destroyRenderer();

            SDL_DestroyWindow(gSdlWindow);
            gSdlWindow = NULL;
#ifdef __3DS__
            GNWSystemError("createRenderer failed\n");
#endif
            return -1;
        }
    }

    return 0;
}

// 0x4CAF9C
int GNW95_init_DirectDraw(int width, int height, int bpp)
{
    if (gSdlSurface != NULL) {
        unsigned char* palette = GNW95_GetPalette();
        GNW95_reset_mode();

        if (GNW95_init_DirectDraw(width, height, bpp) == -1) {
            return -1;
        }

        GNW95_SetPalette(palette);

        return 0;
    }

    gSdlSurface = SDL_CreateRGBSurface(0, width, height, bpp, 0, 0, 0, 0);

    SDL_Color colors[256];
    for (int index = 0; index < 256; index++) {
        colors[index].r = index;
        colors[index].g = index;
        colors[index].b = index;
        colors[index].a = 255;
    }

    SDL_SetPaletteColors(gSdlSurface->format->palette, colors, 0, 256);

    return 0;
}

// 0x4CB1B0
void GNW95_reset_mode()
{
    if (gSdlSurface != NULL) {
        SDL_FreeSurface(gSdlSurface);
        gSdlSurface = NULL;
    }
}

// 0x4CB310
void GNW95_SetPaletteEntries(unsigned char* palette, int start, int count)
{
    if (gSdlSurface != NULL && gSdlSurface->format->palette != NULL) {
        SDL_Color colors[256];

        if (count != 0) {
            for (int index = 0; index < count; index++) {
                colors[index].r = palette[index * 3] << 2;
                colors[index].g = palette[index * 3 + 1] << 2;
                colors[index].b = palette[index * 3 + 2] << 2;
                colors[index].a = 255;
            }
        }

        SDL_SetPaletteColors(gSdlSurface->format->palette, colors, start, count);
        SDL_BlitSurface(gSdlSurface, NULL, gSdlTextureSurface, NULL);
    }
}

// 0x4CB568
void GNW95_SetPalette(unsigned char* palette)
{
    if (gSdlSurface != NULL && gSdlSurface->format->palette != NULL) {
        SDL_Color colors[256];

        for (int index = 0; index < 256; index++) {
            colors[index].r = palette[index * 3] << 2;
            colors[index].g = palette[index * 3 + 1] << 2;
            colors[index].b = palette[index * 3 + 2] << 2;
            colors[index].a = 255;
        }

        SDL_SetPaletteColors(gSdlSurface->format->palette, colors, 0, 256);
        SDL_BlitSurface(gSdlSurface, NULL, gSdlTextureSurface, NULL);
    }
}

// 0x4CB68C
unsigned char* GNW95_GetPalette()
{
    static unsigned char palette[768];

    if (gSdlSurface != NULL && gSdlSurface->format->palette != NULL) {
        SDL_Color* colors = gSdlSurface->format->palette->colors;

        for (int index = 0; index < 256; index++) {
            SDL_Color* color = &(colors[index]);
            palette[index * 3] = color->r >> 2;
            palette[index * 3 + 1] = color->g >> 2;
            palette[index * 3 + 2] = color->b >> 2;
        }
    }

    return palette;
}

// 0x4CB850
void GNW95_ShowRect(unsigned char* src, unsigned int srcPitch, unsigned int a3, unsigned int srcX, unsigned int srcY, unsigned int srcWidth, unsigned int srcHeight, unsigned int destX, unsigned int destY)
{
    buf_to_buf(src + srcPitch * srcY + srcX, srcWidth, srcHeight, srcPitch, (unsigned char*)gSdlSurface->pixels + gSdlSurface->pitch * destY + destX, gSdlSurface->pitch);

    SDL_Rect srcRect;
    srcRect.x = destX;
    srcRect.y = destY;
    srcRect.w = srcWidth;
    srcRect.h = srcHeight;

    SDL_Rect destRect;
    destRect.x = destX;
    destRect.y = destY;
    SDL_BlitSurface(gSdlSurface, &srcRect, gSdlTextureSurface, &destRect);
}

int screenGetWidth()
{
    // TODO: Make it on par with _xres;
    return rectGetWidth(&scr_size);
}

int screenGetHeight()
{
    // TODO: Make it on par with _yres.
    return rectGetHeight(&scr_size);
}

int screenGetVisibleHeight()
{
    return screenGetHeight() - INTERFACE_BAR_HEIGHT;
}

static bool createRenderer(int width, int height)
{
#ifdef __3DS__
    gSdlRenderer = SDL_CreateRenderer(gSdlWindow, -1, 0);
    gSdlRenderer2 = SDL_CreateRenderer(gSdlWindow2, -1, 0);
#else
    gSdlRenderer = SDL_CreateRenderer(gSdlWindow, -1, 0);
#endif
    if (gSdlRenderer == NULL) {
        return false;
    }
#ifdef __3DS__
    if (SDL_RenderSetLogicalSize(gSdlRenderer, 400, 240) != 0) {
        return false;
    }
#else
    if (SDL_RenderSetLogicalSize(gSdlRenderer, width, height) != 0) {
        return false;
    }
#endif
    gSdlTexture = SDL_CreateTexture(gSdlRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (gSdlTexture == NULL) {
        return false;
    }

    Uint32 format;
    if (SDL_QueryTexture(gSdlTexture, &format, NULL, NULL, NULL) != 0) {
        return false;
    }

    gSdlTextureSurface = SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
    if (gSdlTextureSurface == NULL) {
        return false;
    }

    return true;
}

static void destroyRenderer()
{
    if (gSdlTextureSurface != NULL) {
        SDL_FreeSurface(gSdlTextureSurface);
        gSdlTextureSurface = NULL;
    }

    if (gSdlTexture != NULL) {
        SDL_DestroyTexture(gSdlTexture);
        gSdlTexture = NULL;
    }

    if (gSdlRenderer != NULL) {
        SDL_DestroyRenderer(gSdlRenderer);
        gSdlRenderer = NULL;
    }
#ifdef __3DS__
    if (gSdlRenderer2 != NULL) {
        SDL_DestroyRenderer(gSdlRenderer2);
        gSdlRenderer2 = NULL;
    }
#endif
}

void handleWindowSizeChanged()
{
    destroyRenderer();
    createRenderer(screenGetWidth(), screenGetHeight());
}

void renderPresent()
{
    SDL_UpdateTexture(gSdlTexture, NULL, gSdlTextureSurface->pixels, gSdlTextureSurface->pitch);
#ifdef __3DS__
/* top screen */
    SDL_Rect sourceRect;
    SDL_Rect destRect;

    SDL_RenderClear(gSdlRenderer);

    switch (ctr_display.active)
    {
        case ctr_display_t::DISPLAY_SPLASH:
        case ctr_display_t::DISPLAY_MAIN:
        case ctr_display_t::DISPLAY_PAUSE:
        case ctr_display_t::DISPLAY_SKILLDEX:
        {
            sourceRect = {    0,   0, 640, 480 };
            destRect   = {   40,   0, 320, 240 };
            break;
        }
        case ctr_display_t::DISPLAY_MOVIE:
        {
			sourceRect = { 105,  80, 430, 320 };
            destRect   = {  40,   0, 320, 240 };
            break;
        }
        case ctr_display_t::DISPLAY_DIALOG:
        {
            std::vector<DisplayRect>& dialogRects = displayRectMap[ctr_display_t::DISPLAY_DIALOG_TOP];
            if (!dialogRects.empty()) {
                const DisplayRect& firstRect = dialogRects.front();
                sourceRect = firstRect.srcRect;
                destRect = firstRect.dstRect;
                SDL_RenderCopy(gSdlRenderer, gSdlTexture, &sourceRect, &destRect);

                const DisplayRect& secondRect = dialogRects[1];
                sourceRect = secondRect.srcRect;
                destRect = secondRect.dstRect;
            }
            break;
        }
        default:
            switch (currentInput)
            {
                case ctr_input_t::INPUT_TOUCH:
                case ctr_input_t::INPUT_QTM:
                case ctr_input_t::INPUT_CPAD:
                {
                    sourceRect = { offsetX, offsetY, 400, 240 };
                    break;
                }
                default:
                    sourceRect = { 0, 0, 400, 240 };
                    break;
            }

            destRect = { 0, 0, 400, 240 };
            break;
    }

    SDL_RenderCopy(gSdlRenderer, gSdlTexture, &sourceRect, &destRect);
    SDL_RenderPresent(gSdlRenderer);

/* bottom screen */
    SDL_BlitSurface(gSdlTextureSurface, &sourceRect2, surface2, &destRect2);
    SDL_Texture* surfaceTexture2 = SDL_CreateTextureFromSurface(gSdlRenderer2, surface2);
    SDL_RenderClear(gSdlRenderer2);

    switch (ctr_display.active)
    {
        case ctr_display_t::DISPLAY_FULL:
        {
            SDL_RenderCopy(gSdlRenderer2, surfaceTexture2, NULL, NULL);
            break;
        }
        case ctr_display_t::DISPLAY_SPLASH:
        case ctr_display_t::DISPLAY_MOVIE:
        {
            break;
        }
        case ctr_display_t::DISPLAY_DIALOG:
        {
            std::vector<DisplayRect>& dialogRects = displayRectMap[ctr_display_t::DISPLAY_DIALOG_BACK];
            if (!dialogRects.empty()) {
                const DisplayRect& firstRect = dialogRects.front();
                sourceRect = firstRect.srcRect;
                destRect = firstRect.dstRect;
                SDL_RenderCopy(gSdlRenderer2, surfaceTexture2, &sourceRect, &destRect);
            }
        }

        default:
            std::vector<DisplayRect>& displayRects = displayRectMap[ctr_display.active];
            for (const DisplayRect& displayRect : displayRects) {
                SDL_RenderCopy(gSdlRenderer2, surfaceTexture2, &displayRect.srcRect, &displayRect.dstRect);
            }
            break;
    }

    SDL_RenderPresent(gSdlRenderer2);
    SDL_DestroyTexture(surfaceTexture2);
#else
    SDL_RenderClear(gSdlRenderer);
    SDL_RenderCopy(gSdlRenderer, gSdlTexture, NULL, NULL);
    SDL_RenderPresent(gSdlRenderer);
#endif
}

} // namespace fallout
