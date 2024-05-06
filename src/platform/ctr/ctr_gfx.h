#ifndef FALLOUT_PLATFORM_CTR_GFX_H_
#define FALLOUT_PLATFORM_CTR_GFX_H_

#include <3ds.h>
#include <citro3d.h>

#include "plib/gnw/svga.h"

#define DISPLAY_TRANSFER_FLAGS                                                                     \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |               \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |  \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define TEXTURE_TRANSFER_FLAGS                                                                     \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |               \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

namespace fallout {

void drawRect(const float subTexX, const float subTexY, const float subTexW, const float subTexH,
        const float posX, const float posY, const float newWidth, const float newHeight);

void ctr_gfx_draw(SDL_Surface* gSdlSurface);

void beginRender(bool vSync);
void drawTopRenderTarget(uint32_t clearColor);
void drawBottomRenderTarget(uint32_t clearColor);
void finishRender();

void initTransferTexture();

void ctr_gfx_init();
void ctr_gfx_exit();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_GFX_H_ */
