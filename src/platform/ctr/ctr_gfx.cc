#include <string.h>
#include <stdio.h>
#include <iostream>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/mouse.h"

namespace fallout {
ctr_display_t ctr_display;

void convertTouchToTextureCoordinates(int tmp_touchX, int tmp_touchY, const TextureInfo* textureInfos, int startTextureInfos, int numTextureInfos, int* originalX, int* originalY) {
    for (int i = (startTextureInfos?startTextureInfos:0); i < (startTextureInfos+numTextureInfos); ++i) {
        const TextureInfo* textureInfo = &textureInfos[i];
        if (tmp_touchX >= textureInfo->dstRect.x && tmp_touchX < (textureInfo->dstRect.x + textureInfo->dstRect.w) &&
            tmp_touchY >= textureInfo->dstRect.y && tmp_touchY < (textureInfo->dstRect.y + textureInfo->dstRect.h)) {
            *originalX = (int)((tmp_touchX - textureInfo->dstRect.x) / (float)(textureInfo->dstRect.w) * textureInfo->srcRect.w + textureInfo->srcRect.x);
            *originalY = (int)((tmp_touchY - textureInfo->dstRect.y) / (float)(textureInfo->dstRect.h) * textureInfo->srcRect.h + textureInfo->srcRect.y);
            return;
        }
    }
    *originalX = tmp_touchX;
    *originalY = tmp_touchY;
}

void setDisplay(ctr_display_t::active_display_t newActive)
{
    ctr_display.previous = ctr_display.active;
    ctr_display.active = newActive;
}

void setPreviousAsCurrent()
{
    std::swap(ctr_display.active, ctr_display.previous);
}

} // namespace fallout
