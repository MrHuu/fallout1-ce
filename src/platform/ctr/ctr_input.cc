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

#define SINGLE_CLICK(x) kHeld & x && (!(oldpad & x))

#define MAX_OFFSET_X (640 - 400)
#define MAX_OFFSET_Y (480 - 240)

#define MAX_OFFSET_X_QTM 120
#define MAX_OFFSET_Y_QTM 120

struct CursorPosition {
    int x;
    int y;
};

CursorPosition absoluteCursorPosition;

QTM_HeadTrackingInfo qtminfo;
qtm_state_t qtm_state;
u32 qtm_pos, qtm_x, qtm_y;
bool qtm_usable;

static u32 oldpad = 0;

bool relativeMode = true;
int speedDivider = 5;
int deadzone = 15;

int offsetX = 0;
int offsetY = 0;
int offsetX_field = 160;
int offsetY_field = 40;

int currentInput = INPUT_TOUCH;

ctr_input_t ctr_input;

void convertTouchToTextureCoordinates(rectMap_e activeDisplayRectMap, int touch_x, int touch_y, int *originalX, int *originalY)
{
    for (int i = 0; i < numRectsInMap[activeDisplayRectMap]; ++i) {

    rectMap_t *dstRect = rectMaps[activeDisplayRectMap][i];

        if (touch_x >= dstRect->dst_x && touch_x < (dstRect->dst_x + dstRect->dst_w) &&
            touch_y >= (240-(dstRect->dst_y+dstRect->dst_h)) && touch_y < (240-dstRect->dst_y)) {

            rectMap_t *srcRect = rectMaps[activeDisplayRectMap][i];

            float x_scale = dstRect->dst_w / srcRect->src_w;
            float y_scale = dstRect->dst_h / srcRect->src_h;

            float adjusted_touch_x = touch_x  - dstRect->dst_x;
            float adjusted_touch_y = touch_y - (240-(dstRect->dst_y+dstRect->dst_h));

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
        case DISPLAY_FIELD:
            convertTouchToTextureCoordinates(ctr_rectMap.active,
                    ctr_input.frame.touchX, ctr_input.frame.touchY, newX, newY);
            *newX += offsetX_field;
            *newY += offsetY_field;
            break;
        case DISPLAY_LOADSAVE:
            convertTouchToTextureCoordinates(DISPLAY_LOADSAVE_SLOT,
                    ctr_input.frame.touchX, ctr_input.frame.touchY, newX, newY);
            *newY += getSaveSlotOffset();
        default:
            convertTouchToTextureCoordinates(ctr_rectMap.active,
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

    int moveX = static_cast<int>(deltaX) / speedDivider;
    int moveY = -static_cast<int>(deltaY) / speedDivider;

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
    if ((device_model == CFG_MODEL_N3DS) || (device_model == CFG_MODEL_N3DSXL))
    {
        qtmInit();
        qtm_usable = qtmCheckInitialized();
    }
    else
        qtm_usable = false;

    if(!qtm_usable) currentInput = INPUT_TOUCH;

    resetCursorPosition();
}

void ctr_exit_qtm()
{
    qtm_usable = false;
    qtmExit();
}

void ctr_process_message()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if ((currentInput==INPUT_TOUCH) &&
                (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP || e.type == SDL_FINGERMOTION)) {
            int touchX = e.tfinger.x * 320;
            int touchY = e.tfinger.y * 240;

            offsetX = (int)touchX * MAX_OFFSET_X / 320;
            offsetY = 240-((int)touchY * MAX_OFFSET_Y / 240);

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
}

void ctr_input_frame()
{
    u32 kHeld = hidKeysHeld();
    ctr_input.frame.kHeld = kHeld;

    if ((SINGLE_CLICK(KEY_A)) || (SINGLE_CLICK(KEY_ZL))) {
        if (ctr_rectMap.fullscreen) {
            ctr_rectMap.fullscreen = false;
            setActiveRectMap(DISPLAY_LAST);
        } else {
            if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI))
            {
                if (ctr_rectMap.active == DISPLAY_FIELD)
                    setActiveRectMap(DISPLAY_GUI);
                else
                    setActiveRectMap(DISPLAY_FIELD);
            }
        }
    }

    if (SINGLE_CLICK(KEY_SELECT)) {
        ctr_rectMap.fullscreen = true;
        setActiveRectMap(DISPLAY_LAST);
    }

    if (SINGLE_CLICK(KEY_Y)) {
//        ctr_rectMap.active = static_cast<rectMap_e>((ctr_rectMap.active + 1) % DISPLAY_LAST);
    }

    if (SINGLE_CLICK(KEY_X)) {
        currentInput = ((currentInput + 1) % (INPUT_LAST));
    }

    if ((ctr_rectMap.active == DISPLAY_FIELD) || (ctr_rectMap.active == DISPLAY_GUI)) {
        if(kHeld & KEY_DUP) {
            if (offsetY_field > 0) {
                offsetY_field -= 20;
            } else {
                map_scroll(0, -1);
            }
        }
        if(kHeld & KEY_DDOWN) {
            if (offsetY_field < 80) {
                offsetY_field+=20;
            } else {
                map_scroll(0, 1);
            }
        }
        if(kHeld & KEY_DLEFT) {
            if (offsetX_field > 0) {
                offsetX_field-=20;
            } else {
                map_scroll(-1, 0);
            }
        }
        if(kHeld & KEY_DRIGHT) {
            if (offsetX_field < 240) {
                offsetX_field+=20;
            } else {
                map_scroll(1, 0);
            }
        }
    } else {
        if(kHeld & KEY_DUP)
            map_scroll(0, -1);
        if(kHeld & KEY_DDOWN)
            map_scroll(0, 1);
        if(kHeld & KEY_DLEFT)
            map_scroll(-1, 0);
        if(kHeld & KEY_DRIGHT)
            map_scroll(1, 0);
    }

    switch (currentInput) {
        case INPUT_TOUCH:
        {
            if(qtm_usable)
                ctr_exit_qtm();

            break;
        }
        case INPUT_CPAD:
        {
            if(qtm_usable)
                ctr_exit_qtm();

            circlePosition circle;
            hidCircleRead(&circle);

            circlePosition cstick;
            hidCstickRead(&cstick);

            updateCursorPosition(circle.dx, circle.dy, speedDivider, relativeMode, deadzone);

            offsetX = absoluteCursorPosition.x;
            offsetY = absoluteCursorPosition.y;
            break;
        }
        case INPUT_QTM:
        {
            if(!qtm_usable)
            {
                ctr_init_qtm();
            }
            else if(qtm_usable)
            {
                float range_x = 320.0f;
                float range_y = 240.0f;

                if (qtm_state.rel_x == 0)
                {
                    qtm_state.rel_x      = 25000;
                    qtm_state.rel_y      = 25000;
                    qtm_state.multiplier = 1;
                }
   
                Result ret = QTM_GetHeadTrackingInfo(0, &qtminfo);
                if(ret==0) {
                    if(qtmCheckHeadFullyDetected(&qtminfo)) {
                        ret = qtmConvertCoordToScreen(&qtminfo.coords0[0], &range_x, &range_y, &qtm_x, &qtm_y);
                    }
                }
            }

            if (SINGLE_CLICK(KEY_ZR)) {
                qtm_state.multiplier += 1;
                if (qtm_state.multiplier > 10)
                    qtm_state.multiplier = 1;
            }

            int rel_qtm_offsetX = (int)(qtm_x - 120);
            int rel_qtm_offsetY = (int)(qtm_y - 120);

            int textureX = rel_qtm_offsetX * qtm_state.multiplier;
            int textureY = rel_qtm_offsetY * qtm_state.multiplier;

            if (textureX > MAX_OFFSET_X_QTM) {
                textureX = MAX_OFFSET_X_QTM;
            } else if (textureX < -MAX_OFFSET_X_QTM) {
                textureX = -MAX_OFFSET_X_QTM;
            }

            if (textureY > MAX_OFFSET_Y_QTM) {
                textureY = MAX_OFFSET_Y_QTM;
            } else if (textureY < -MAX_OFFSET_Y_QTM) {
                textureY = -MAX_OFFSET_Y_QTM;
            }

            offsetX = 120 + textureX;
            offsetY = 120 + textureY;
            break;
        }
    }
    oldpad = kHeld;
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
}

void ctr_input_exit()
{
	ctr_exit_qtm();
}

} // namespace fallout
