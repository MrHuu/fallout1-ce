#include <string.h>
#include <stdio.h>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/mouse.h"

namespace fallout {

ctr_input_t ctr_input;

float touchX = 0.0f;
float touchY = 0.0f;

int lastTouchX = 0;
int lastTouchY = 0;

int offsetX = 0;
int offsetY = 0;

u32 qtm_pos;
u32 qtm_x, qtm_y;
Result ret;
bool qtm_usable;
QTM_HeadTrackingInfo qtminfo;

float qtm_offsetX;
float qtm_offsetY;

qtm_state_t qtm_state;
int currentInput = ctr_input_t::INPUT_TOUCH;

static uint32_t oldpad = 0;
u32 kHeld;


void ctr_init_qtm()
{
    qtmInit();
    qtm_usable = qtmCheckInitialized();
    if(!qtm_usable) currentInput = ctr_input_t::INPUT_TOUCH;
}

void ctr_exit_qtm()
{
    qtm_usable = false;
    qtmExit();
}




void ctr_input_frame()
{
    kHeld = hidKeysHeld();

    if (SINGLE_CLICK(KEY_A)) {
        if (ctr_display.active == ctr_display_t::DISPLAY_GUI)
            ctr_display.active = ctr_display_t::DISPLAY_FIELD;
        else
            ctr_display.active = ctr_display_t::DISPLAY_GUI;
    }

    if (SINGLE_CLICK(KEY_B)) {
        ctr_display.active = ctr_display_t::DISPLAY_FULL;
    }

    if (SINGLE_CLICK(KEY_Y)) {
        ctr_display.active = static_cast<ctr_display_t::active_display_t>((ctr_display.active + 1) % ctr_display_t::DISPLAY_LAST);
    }

    if (SINGLE_CLICK(KEY_X)) {
        currentInput = static_cast<ctr_input_t::active_input_t>((currentInput + 1) % (ctr_input_t::INPUT_LAST));
    }

    switch (currentInput) {
        case ctr_input_t::INPUT_TOUCH:
            if(qtm_usable)
                ctr_exit_qtm();
            break;
        case ctr_input_t::INPUT_QTM:
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
   
                ret = QTM_GetHeadTrackingInfo(0, &qtminfo);
                if(ret==0) {
                    if(qtmCheckHeadFullyDetected(&qtminfo)) {
                        ret = qtmConvertCoordToScreen(&qtminfo.coords0[0], &range_x, &range_y, &qtm_x, &qtm_y);
                    }
                }
            }

            if (SINGLE_CLICK(KEY_ZR))
			{
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

            qtm_offsetX = 120 + textureX;
            qtm_offsetY = 120 + textureY;
/*
            if (SINGLE_CLICK(KEY_ZL)) {
                char inputBuffer[512] = ""; // Buffer to store input data

                snprintf(inputBuffer, 512, "qtm X: %ld, Y: %ld\noffset X: %f, Y: %f", qtm_x,qtm_y,qtm_offsetX, qtm_offsetY);
                GNWSystemError(inputBuffer);
            }
*/
            break;
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
