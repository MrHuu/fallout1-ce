#include <iostream>

#include "ctr_input.h"
#include "ctr_gfx.h"

namespace fallout {

ctr_display_t ctr_display;
std::map<ctr_display_t::active_display_t, std::vector<DisplayRect>> displayRectMap;

void convertTouchToTextureCoordinates(int tmp_touchX, int tmp_touchY, ctr_display_t::active_display_t displayType, int* originalX, int* originalY) {
    const std::vector<DisplayRect>& displayRects = displayRectMap[displayType];

    for (const DisplayRect& displayRect : displayRects) {
        if (tmp_touchX >= displayRect.dstRect.x && tmp_touchX < (displayRect.dstRect.x + displayRect.dstRect.w) &&
            tmp_touchY >= displayRect.dstRect.y && tmp_touchY < (displayRect.dstRect.y + displayRect.dstRect.h)) {
            *originalX = static_cast<int>((tmp_touchX - displayRect.dstRect.x) / static_cast<float>(displayRect.dstRect.w) * displayRect.srcRect.w + displayRect.srcRect.x);
            *originalY = static_cast<int>((tmp_touchY - displayRect.dstRect.y) / static_cast<float>(displayRect.dstRect.h) * displayRect.srcRect.h + displayRect.srcRect.y);
            return;
        }
    }
    *originalX = tmp_touchX;
    *originalY = tmp_touchY;
}

void initializeDisplayRectMap() {
    displayRectMap[ctr_display_t::DISPLAY_FULL] = {
        {{   0,   0, 640, 480}, {40, 0, 320, 240}}
    };

    displayRectMap[ctr_display_t::DISPLAY_FIELD] = {
        {{  0,   0, 640, 380}, {  0,   0, 320, 190}}
    };

    displayRectMap[ctr_display_t::DISPLAY_GUI] = {
        {{  5, 380, 200, 100}, {  0,  10, 200, 100}},
        {{200, 380, 320, 100}, {  0, 140, 320, 100}},
        {{520, 380, 140, 100}, {200,  10, 120, 100}}
    };

    displayRectMap[ctr_display_t::DISPLAY_MOVIE] = {
        {{  0,   0, 640, 480}, {  0,   0, 320, 240}}
    };

    displayRectMap[ctr_display_t::DISPLAY_MAIN] = {
        {{400,  25, 215, 220}, { 55,   0, 215, 240}}
    };

    displayRectMap[ctr_display_t::DISPLAY_PAUSE] = {
        {{240,  72, 160, 215}, { 55,   0, 215, 240}}
    };

    displayRectMap[ctr_display_t::DISPLAY_PAUSE_CONFIRM] = {
        {{170, 120, 300, 120}, {  0,  50, 320, 120}}
    };

    displayRectMap[ctr_display_t::DISPLAY_DIALOG_TOP] = {
        {{ 80,   0, 480, 290}, {  0,   0, 397, 240}},
        {{130, 230, 390,  60}, { 10, 180, 380,  60}}   // dialog
    };

    displayRectMap[ctr_display_t::DISPLAY_DIALOG_BACK] = {
        {{560, 440,  80,  36}, {  0, 160, 320,  80}}
    };

    displayRectMap[ctr_display_t::DISPLAY_DIALOG] = {
        {{130, 320, 385, 160}, {  0,   0, 320, 160}},     // bottom dialog
        {{  5, 410,  70,  70}, { 55, 160,  70,  70}},     // bottom review
        {{565, 300,  70,  60}, {125, 175,  70,  60}},     // bottom barter
        {{565, 360,  70,  80}, {195, 160,  70,  80}}      // bottom about

    };

    displayRectMap[ctr_display_t::DISPLAY_SKILLDEX] = {
        {{452,   5, 240, 190}, {  0,   0, 160, 190}},
        {{452, 190, 240, 180}, {160,  40, 160, 180}}
    };
}

void setActiveDisplay(ctr_display_t::active_display_t displayType) {
    ctr_display.previous = ctr_display.active;
    ctr_display.active = displayType;
}

void setPreviousAsCurrent()
{
    ctr_display.active = ctr_display.previous;
}

} // namespace fallout
