#include <string.h>
#include <stdio.h>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "ctr_rectmap.h"

#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/mouse.h"
#include "plib/gnw/touch.h"
#include "game/map.h"

namespace fallout {

#define FLAG_NONE 0x00
#define FLAG_SELECT_DDOWN 0x01
#define FLAG_SELECT_DUP 0x02
#define FLAG_SELECT_DLEFT 0x04
#define FLAG_SELECT_DRIGHT 0x08
#define FLAG_START_DDOWN 0x10
#define FLAG_START_DUP 0x20
#define FLAG_START_DLEFT 0x40
#define FLAG_START_DRIGHT 0x80

#define KEY_PRESS(x) ctr_input.frame.kHeld & x && (!(oldpad & x))
#define KEY_HELD(x) (ctr_input.frame.kHeld & x)
#define KEY_RELEASED(x) (!(ctr_input.frame.kHeld & x) && (oldpad & x))

#define MAX_OFFSET 240
#define MAX_OFFSET_QTM 120

struct CursorPosition {
    int x;
    int y;
};

CursorPosition absoluteCursorPosition;
ctr_input_t ctr_input;

static u8 key_combi_flags = FLAG_NONE;
static u32 oldpad = 0;
static bool scaled_rect_top = false;
static bool relativeMode = true;
static float previousSliderState = -1.0f;
static float manualScaleFactor = 0.0f;
static int speedDivider = 5;
static int deadzone = 15;

int offsetX = 0;
int offsetY = 0;

void input_touch_to_texture(rectMap_e activeDisplayRectMap, int touch_x, int touch_y, int *originalX, int *originalY)
{
    for (int i = 0; i < numRectsInMap[activeDisplayRectMap]; ++i) {

    rectMap_t *dstRect = rectMaps[activeDisplayRectMap][i];

        if (touch_x >= dstRect->dst_x && touch_x < (dstRect->dst_x + dstRect->dst_w) &&
            touch_y >= (240 - (dstRect->dst_y + dstRect->dst_h)) && touch_y < (240 - dstRect->dst_y)) {

            rectMap_t *srcRect = rectMaps[activeDisplayRectMap][i];

            float x_scale = dstRect->dst_w / srcRect->src_w;
            float y_scale = dstRect->dst_h / srcRect->src_h;

            float adjusted_touch_x = touch_x - dstRect->dst_x;
            float adjusted_touch_y = touch_y - (240 - (dstRect->dst_y + dstRect->dst_h));

            *originalX = (adjusted_touch_x / x_scale) + srcRect->src_x;
            *originalY = (adjusted_touch_y / y_scale) + srcRect->src_y;
        }
    }
}

void ctr_input_get_touch(int *newX, int *newY)
{
    switch (ctr_rectMap.active)
    {
        case DISPLAY_FULL:
            *newX = (ctr_input.frame.touchX * screenGetWidth()) / 320;
            *newY = (ctr_input.frame.touchY * screenGetHeight()) / 240;

            if (*newX < 15) *newX = 0;
            if (*newY < 15) *newY = 0;

            if (*newX > 625) *newX = 640;
            if (*newY > 465) *newY = 480;
            break;
        case DISPLAY_LOADSAVE:
            input_touch_to_texture(DISPLAY_LOADSAVE_SLOT,
                    ctr_input.frame.touchX, ctr_input.frame.touchY, newX, newY);
            *newY += getSaveSlotOffset();
        default:
            input_touch_to_texture(ctr_rectMap.active,
                    ctr_input.frame.touchX, ctr_input.frame.touchY, newX, newY);
            break;
    }
}

void updateCursorPosition(s16 deltaX, s16 deltaY, int speedDivider, bool relativeMode, int deadzone)
{
    const int screenWidth = 240;
    const int screenHeight = 240;

    int cursorX = absoluteCursorPosition.x;
    int cursorY = absoluteCursorPosition.y;

    int moveX = (int)deltaX / speedDivider;
    int moveY = -(int)deltaY / speedDivider;

    if (abs(moveX) < deadzone) {
        moveX = 0;
    }
    if (abs(moveY) < deadzone) {
        moveY = 0;
    }

    if (relativeMode) {
        cursorX += moveX;
        cursorY += moveY;
    } else {
        cursorX = moveX;
        cursorY = moveY;
    }

    if (cursorX < 0) {
        cursorX = 0;
    } else if (cursorX >= screenWidth) {
        cursorX = screenWidth - 1;
    }

    if (cursorY < 0) {
        cursorY = 0;
    } else if (cursorY >= screenHeight) {
        cursorY = screenHeight - 1;
    }

    absoluteCursorPosition.x = cursorX;
    absoluteCursorPosition.y = cursorY;
}

void resetCursorPosition()
{
    absoluteCursorPosition.x = 120;
    absoluteCursorPosition.y = 120;
}

void ctr_init_qtm()
{
    u8 device_model = 0xFF;
    CFGU_GetSystemModel(&device_model);
    if ((device_model == CFG_MODEL_N3DS) || (device_model == CFG_MODEL_N3DSXL)) {
        qtmInit();
        ctr_input.qtm_state.qtm_usable = qtmCheckInitialized();
    } else
        ctr_input.qtm_state.qtm_usable = false;

    if (!ctr_input.qtm_state.qtm_usable) ctr_input.input = INPUT_TOUCH;

    resetCursorPosition();
}

void ctr_exit_qtm()
{
    ctr_input.qtm_state.qtm_usable = false;
    qtmExit();
}

void ctr_input_process()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if ((ctr_input.input == INPUT_TOUCH) &&
                (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP || e.type == SDL_FINGERMOTION)) {
            float touchX = e.tfinger.x * 320;
            float touchY = e.tfinger.y * 240;

            offsetX = touchX * MAX_OFFSET / 320;
            offsetY = 240 - (touchY * MAX_OFFSET / 240);

            ctr_input.frame.touchX = touchX;
            ctr_input.frame.touchY = touchY;
        }

        switch (e.type) {
        case SDL_FINGERDOWN:
            touch_handle_start(&(e.tfinger));
            break;
        case SDL_FINGERMOTION:
            touch_handle_move(&(e.tfinger));
            break;
        case SDL_FINGERUP:
            touch_handle_end(&(e.tfinger));
            break;

        case SDL_QUIT:
            exit(EXIT_SUCCESS);
            break;
        }
    }
    touch_process_gesture();

    ctr_input.frame.kHeld = hidKeysHeld();

    if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
        float scaleFactorToUse;
        float currentSliderState = osGet3DSliderState();
        int sliderChanged = (currentSliderState != previousSliderState);

        if (key_combi_flags & (FLAG_SELECT_DDOWN | FLAG_SELECT_DUP)) {
            scaleFactorToUse = manualScaleFactor;
        } else if (sliderChanged) {
            scaleFactorToUse = currentSliderState;
            previousSliderState = currentSliderState;
        } else if (scaled_rect_top == (ctr_rectMap.active == DISPLAY_GUI)) {
            return;
        } else {
            scaleFactorToUse = -1.0f;
        }
        scaled_rect_top = (ctr_rectMap.active == DISPLAY_GUI);

        if (setRectMapScaled(scaled_rect_top, scaleFactorToUse) == 0) {
            mouse_hide();
            mouse_set_position(offsetX_field + (rectMaps[DISPLAY_FIELD][0]->src_w / 2),
                    offsetY_field + (rectMaps[DISPLAY_FIELD][0]->src_h / 2));
            mouse_show();
        }
    }
}

void input_frame_touch()
{
    return;
}

void input_frame_axis(int *delta_x, int *delta_y)
{
    int x, y;
    int moveX;
    int moveY;

    if ((ctr_rectMap.active == DISPLAY_FULL) || (ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {

        circlePosition circle;
        hidCircleRead(&circle);

        circlePosition cstick;
        hidCstickRead(&cstick);

        // both axis for now
        moveX = (int)circle.dx / 15 + (int)cstick.dx / 15;
        moveY = -((int)circle.dy / 15 + (int)cstick.dy / 15);

        if (abs(moveX) < 1) {
            moveX = 0;
        }
        if (abs(moveY) < 1) {
            moveY = 0;
        }

        if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
            if ((moveX != 0) || (moveY != 0)) {
                mouse_get_position(&x, &y);
                if (y > 380) {
                    mouse_hide();
                    mouse_set_position(320, 190);
                    mouse_show();
                } else if (y + moveY > 380) {
                    mouse_hide();
                    mouse_set_position(x, 380);
                    mouse_show();
                } else {
                    *delta_x += moveX;
                    *delta_y += moveY;
                }
            }
        } else {
            *delta_x += moveX;
            *delta_y += moveY;
        }
    }
}

void input_frame_buttons(int *delta_x, int *delta_y, int *buttons)
{
    if (KEY_HELD(KEY_SELECT) && KEY_PRESS(KEY_DDOWN)) {
        manualScaleFactor -= 0.2f; // zoom out
        if (manualScaleFactor < 0.0f) {
            manualScaleFactor = 0.0f;
        }
        key_combi_flags |= FLAG_SELECT_DDOWN;
    }

    if (KEY_HELD(KEY_SELECT) && KEY_PRESS(KEY_DUP)) {
        manualScaleFactor += 0.2f; // zoom in
        if (manualScaleFactor > 1.0f) {
            manualScaleFactor = 1.0f;
        }
        key_combi_flags |= FLAG_SELECT_DUP;
    }

    if (KEY_RELEASED(KEY_SELECT)) {
        if (key_combi_flags) {
            key_combi_flags = FLAG_NONE;
        } else {
            ctr_input.mode = ((ctr_input.mode + 1) % DISPLAY_MODE_FULL_TOP); // DISPLAY_MODE_LAST
            setActiveRectMap(DISPLAY_LAST);
        }
    }

    if (key_combi_flags == FLAG_NONE) {
        if (KEY_PRESS(KEY_L)) {
            *buttons |= MOUSE_STATE_RIGHT_BUTTON_DOWN;
        }

        if ((ctr_rectMap.active == DISPLAY_FULL) ||
                (ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
            if (KEY_PRESS(KEY_R)) {
                *buttons |= MOUSE_STATE_LEFT_BUTTON_DOWN;
            }
        }

        if ((KEY_PRESS(KEY_A)) || (KEY_PRESS(KEY_ZL))) {
            if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
                if (ctr_rectMap.active == DISPLAY_FIELD)
                    setActiveRectMap(DISPLAY_GUI);
                else
                    setActiveRectMap(DISPLAY_FIELD);
            }
        }

        if (KEY_PRESS(KEY_Y)) {
            // ctr_input.mode = ((ctr_input.mode + 1) % DISPLAY_MODE_LAST);
        }

        if (KEY_PRESS(KEY_X)) {
            // currentInput = ((currentInput + 1) % (INPUT_LAST));
        }

        if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
            offsetX_field = rectMaps[DISPLAY_FIELD][0]->src_x;
            offsetY_field = rectMaps[DISPLAY_FIELD][0]->src_y;

            if (KEY_HELD(KEY_DUP)) {
                if (offsetY_field > 0) {
                    int deltaY = (offsetY_field >= 20) ? 20 : offsetY_field;
                    offsetY_field -= deltaY;
                    if (ctr_rectMap.active != DISPLAY_GUI) {
                        *delta_y -= deltaY;
                    } else {
                        int x, y;
                        mouse_get_position(&x, &y);
                        if (y < 380) {
                            *delta_y -= deltaY;
                        }
                    }
                } else {
                    map_scroll(0, -1);
                }
            }
            if (KEY_HELD(KEY_DDOWN)) {
                if (offsetY_field < offsetY_field_scaled_max) {
                    int deltaY = (offsetY_field_scaled_max - offsetY_field >= 20) ?
                                 20 : offsetY_field_scaled_max - offsetY_field;
                    offsetY_field += deltaY;
                    if (ctr_rectMap.active != DISPLAY_GUI) {
                        *delta_y += deltaY;
                    } else {
                        int x, y;
                        mouse_get_position(&x, &y);
                        if (y < 380) {
                            *delta_y += deltaY;
                        }
                    }
                } else {
                    map_scroll(0, 1);
                }
            }
            if (KEY_HELD(KEY_DLEFT)) {
                if (offsetX_field > 0) {
                    int deltaX = (offsetX_field >= 20) ? 20 : offsetX_field;
                    offsetX_field -= deltaX;
                    if (ctr_rectMap.active != DISPLAY_GUI) {
                        *delta_x -= deltaX;
                    } else {
                        int x, y;
                        mouse_get_position(&x, &y);
                        if (y < 380) {
                            *delta_x -= deltaX;
                        }
                    }
                } else {
                    map_scroll(-1, 0);
                }
            }
            if (KEY_HELD(KEY_DRIGHT)) {
                if (offsetX_field < offsetX_field_scaled_max) {
                    int deltaX = (offsetX_field_scaled_max - offsetX_field >= 20) ?
                                 20 : offsetX_field_scaled_max - offsetX_field;
                    offsetX_field += deltaX;
                    if (ctr_rectMap.active != DISPLAY_GUI) {
                        *delta_x += deltaX;
                    } else {
                        int x, y;
                        mouse_get_position(&x, &y);
                        if (y < 380) {
                            *delta_x += deltaX;
                        }
                    }
                } else {
                    map_scroll(1, 0);
                }
            }
            rectMaps[DISPLAY_FIELD][0]->src_x = offsetX_field;
            rectMaps[DISPLAY_FIELD][0]->src_y = offsetY_field;
        } else {
            if (KEY_HELD(KEY_DUP))
                map_scroll(0, -1);
            if (KEY_HELD(KEY_DDOWN))
                map_scroll(0, 1);
            if (KEY_HELD(KEY_DLEFT))
                map_scroll(-1, 0);
            if (KEY_HELD(KEY_DRIGHT))
            map_scroll(1, 0);
        }
    }
    oldpad = ctr_input.frame.kHeld;
}

int ctr_input_frame()
{
    int delta_x = 0;
    int delta_y = 0;
    int buttons = 0;

    switch (ctr_input.input) {
        case INPUT_TOUCH:
            if (ctr_input.qtm_state.qtm_usable)
                ctr_exit_qtm();
            break;

        case INPUT_CPAD:
            if (ctr_input.qtm_state.qtm_usable)
                ctr_exit_qtm();
            circlePosition circle;
            hidCircleRead(&circle);
            updateCursorPosition(circle.dx, circle.dy, speedDivider, relativeMode, deadzone);
            offsetX = absoluteCursorPosition.x;
            offsetY = absoluteCursorPosition.y;
            break;

        case INPUT_QTM:
        {
            if (!ctr_input.qtm_state.qtm_usable) {
                ctr_init_qtm();
            } else if (ctr_input.qtm_state.qtm_usable) {
                float range_x = 320.0f;
                float range_y = 240.0f;
                if (ctr_input.qtm_state.rel_x == 0) {
                    ctr_input.qtm_state.rel_x = 25000;
                    ctr_input.qtm_state.rel_y = 25000;
                    ctr_input.qtm_state.multiplier = 1;
                }
                Result ret = QTM_GetHeadTrackingInfo(0, &ctr_input.qtm_state.qtminfo);
                if (ret == 0) {
                    if (qtmCheckHeadFullyDetected(&ctr_input.qtm_state.qtminfo)) {
                        ret = qtmConvertCoordToScreen(&ctr_input.qtm_state.qtminfo.coords0[0],
                                &range_x, &range_y, &ctr_input.qtm_state.qtm_x, &ctr_input.qtm_state.qtm_y);
                    }
                }
            }
            if (KEY_PRESS(KEY_ZR)) {
                ctr_input.qtm_state.multiplier += 1;
                if (ctr_input.qtm_state.multiplier > 10)
                    ctr_input.qtm_state.multiplier = 1;
            }
            int rel_qtm_offsetX = (int)(ctr_input.qtm_state.qtm_x - 120);
            int rel_qtm_offsetY = (int)(ctr_input.qtm_state.qtm_y - 120);
            int textureX = rel_qtm_offsetX * ctr_input.qtm_state.multiplier;
            int textureY = rel_qtm_offsetY * ctr_input.qtm_state.multiplier;
            if (textureX > MAX_OFFSET_QTM) {
                textureX = MAX_OFFSET_QTM;
            } else if (textureX < -MAX_OFFSET_QTM) {
                textureX = -MAX_OFFSET_QTM;
            }
            if (textureY > MAX_OFFSET_QTM) {
                textureY = MAX_OFFSET_QTM;
            } else if (textureY < -MAX_OFFSET_QTM) {
                textureY = -MAX_OFFSET_QTM;
            }
            offsetX = 120 + textureX;
            offsetY = 120 + textureY;
            break;
        }

        default:
            break;
    }

    input_frame_touch();
    input_frame_axis(&delta_x, &delta_y);
    input_frame_buttons(&delta_x, &delta_y, &buttons);

    if ((delta_x != 0) || (delta_y != 0) || (buttons != 0)) {
        mouse_simulate_input(delta_x, delta_y, buttons);
        return 1;
    }

    return 0;
}

int ctr_input_swkbd(const char *hintText, const char *inText, char *outText)
{
    SwkbdState swkbd;
    static SwkbdStatusData swkbdStatus;
    static SwkbdLearningData swkbdLearning;
    char mybuf[128];

    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    swkbdSetInitialText(&swkbd, inText);
    swkbdSetHintText(&swkbd, hintText);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Done", true);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Close", false);
    swkbdSetFeatures(&swkbd,
            SWKBD_PREDICTIVE_INPUT | SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER);

    static bool reload = false;
    swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
    swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
    reload = true;

    SwkbdButton button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));

    if (button == SWKBD_BUTTON_LEFT) {
        strcpy(outText, mybuf);
        return 1;
    } else if (button == SWKBD_BUTTON_RIGHT)
        return -1;

    printf("swkbd event: %d\n", swkbdGetResult(&swkbd));
    return -2;
}

void ctr_input_init()
{
    ctr_init_qtm();
    ctr_input.input = INPUT_TOUCH;
    previousSliderState = osGet3DSliderState();

    if (setRectMapScaled(scaled_rect_top, 0) == 0) {
        mouse_hide();
        mouse_set_position(offsetX_field + (rectMaps[DISPLAY_FIELD][0]->src_w / 2),
                           offsetY_field + (rectMaps[DISPLAY_FIELD][0]->src_h / 2));
        mouse_show();
    }

}

void ctr_input_exit()
{
    ctr_exit_qtm();
}

} // namespace fallout
