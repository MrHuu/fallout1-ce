#ifndef FALLOUT_PLATFORM_CTR_GFX_H_
#define FALLOUT_PLATFORM_CTR_GFX_H_

#include "plib/gnw/svga.h"

namespace fallout {

#include <3ds.h>

//extern int currentDisplay;
//extern int currentInput;

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
        DISPLAY_LAST
    };
    active_display_t active;
    active_display_t previous;
};
extern ctr_display_t ctr_display;


void convertTouchToTextureCoordinates(int tmp_touchX, int tmp_touchY, const TextureInfo* textureInfos, int startTextureInfos, int numTextureInfos, int* originalX, int* originalY);

void setDisplay(ctr_display_t::active_display_t newActive);
void setPreviousAsCurrent();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_GFX_H_ */
