#include <string.h>
#include <stdio.h>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/mouse.h"
#include "game/map.h"

namespace fallout {

ctr_input_t ctr_input;

int offsetX = 0;
int offsetY = 0;

u32 qtm_pos;
u32 qtm_x, qtm_y;
bool qtm_usable;
QTM_HeadTrackingInfo qtminfo;

qtm_state_t qtm_state;
int currentInput = ctr_input_t::INPUT_TOUCH;

static uint32_t oldpad = 0;
u32 kHeld;

int speedDivider = 2;
int deadzone = 15;
bool relativeMode = true;

void resetCursorPosition();

void ctr_init_qtm()
{
    qtmInit();
    qtm_usable = qtmCheckInitialized();
    if(!qtm_usable) currentInput = ctr_input_t::INPUT_TOUCH;

resetCursorPosition();
}

void ctr_exit_qtm()
{
    qtm_usable = false;
    qtmExit();
}

struct CursorPosition {
    int x;
    int y;
};

CursorPosition absoluteCursorPosition;

void resetCursorPosition() {
    absoluteCursorPosition.x = 120;
    absoluteCursorPosition.y = 120;
}

void updateCursorPosition(s16 deltaX, s16 deltaY, int speedDivider, bool relativeMode, int deadzone) {
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

void ctr_input_frame()
{
    kHeld = hidKeysHeld();

    if ((SINGLE_CLICK(KEY_A)) || (SINGLE_CLICK(KEY_ZL))) {
        if (ctr_display.active == ctr_display_t::DISPLAY_GUI)
            setActiveDisplay(ctr_display_t::DISPLAY_FULL);
        else
            setActiveDisplay(ctr_display_t::DISPLAY_GUI);
    }

    if (SINGLE_CLICK(KEY_B)) {
        setActiveDisplay(ctr_display_t::DISPLAY_FULL);
    }

    if (SINGLE_CLICK(KEY_Y)) {
//        ctr_display.active = static_cast<ctr_display_t::active_display_t>((ctr_display.active + 1) % ctr_display_t::DISPLAY_LAST);
    }

    if (SINGLE_CLICK(KEY_X)) {
        currentInput = static_cast<ctr_input_t::active_input_t>((currentInput + 1) % (ctr_input_t::INPUT_LAST));
    }

    if(kHeld & KEY_DUP)
        map_scroll(0, -1);
    if(kHeld & KEY_DDOWN)
        map_scroll(0, 1);
    if(kHeld & KEY_DLEFT)
        map_scroll(-1, 0);
    if(kHeld & KEY_DRIGHT)
        map_scroll(1, 0);

    switch (currentInput) {
        case ctr_input_t::INPUT_TOUCH:
        {
            if(qtm_usable)
                ctr_exit_qtm();

            break;
        }
        case ctr_input_t::INPUT_CPAD:
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
        case ctr_input_t::INPUT_QTM:
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

int ctr_sys_swkbd(const char *hintText, const char *inText, char *outText)
{
   SwkbdState swkbd;
   static SwkbdStatusData swkbdStatus;
   static SwkbdLearningData swkbdLearning;
   char mybuf[128];

   swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
   swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
   swkbdSetInitialText(&swkbd, inText);
   swkbdSetHintText(&swkbd, hintText);
   swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Close", false);
   swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Done", true);
   swkbdSetFeatures(&swkbd,
         SWKBD_PREDICTIVE_INPUT | SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER);

   static bool reload = false;
   swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
   swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
   reload = true;

   SwkbdButton button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));

   if (button == SWKBD_BUTTON_LEFT)
      return -1;

   else if (button == SWKBD_BUTTON_MIDDLE)
      return 0;

   else if (button == SWKBD_BUTTON_RIGHT)
   {
      strcpy(outText, mybuf);
      return 1;
   }

   printf("swkbd event: %d\n", swkbdGetResult(&swkbd));
   return -2;
}

} // namespace fallout
