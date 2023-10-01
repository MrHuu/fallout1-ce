#ifndef FALLOUT_PLATFORM_CTR_GFX_H_
#define FALLOUT_PLATFORM_CTR_GFX_H_

#include <3ds.h>

#include <map>
#include <vector>
#include <SDL_rect.h>

#include "plib/gnw/svga.h"

namespace fallout {

struct ctr_display_t {
    enum active_display_t {
        DISPLAY_SPLASH = 0,
        DISPLAY_FULL,
        DISPLAY_FIELD,
        DISPLAY_GUI,
        DISPLAY_MOVIE,
        DISPLAY_SKILLDEX,
        DISPLAY_AUTOMAP,
        DISPLAY_PIPBOY,
        DISPLAY_MAIN,
        DISPLAY_PAUSE,
        DISPLAY_PAUSE_CONFIRM,
        DISPLAY_DIALOG,
        DISPLAY_DIALOG_TOP,
        DISPLAY_DIALOG_BACK,
        DISPLAY_LAST
    };
    active_display_t active;
    active_display_t previous;
};
extern ctr_display_t ctr_display;

struct DisplayRect {
    SDL_Rect srcRect;
    SDL_Rect dstRect;
};
extern std::map<ctr_display_t::active_display_t, std::vector<DisplayRect>> displayRectMap;

void initializeDisplayRectMap();
void addTextureInfo(TextureInfo** textureInfos, int* numTextureInfos, ctr_display_t::active_display_t displayType);
void setActiveDisplay(ctr_display_t::active_display_t displayType);

void convertTouchToTextureCoordinates(int tmp_touchX, int tmp_touchY, ctr_display_t::active_display_t displayType, int* originalX, int* originalY);

void setDisplay(ctr_display_t::active_display_t newActive);
void setPreviousAsCurrent();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_GFX_H_ */
