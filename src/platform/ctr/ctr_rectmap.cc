#include <iostream>
#include <stdlib.h>

#include "ctr_rectmap.h"

#include "plib/gnw/gnw.h"
#include "game/loadsave.h"

namespace fallout {

rectMap_t ***rectMaps = { NULL };
int *numRectsInMap = { NULL };

int indicatorSlotNum = 0;

int saveSlotCursor = 4;
float saveSlotOffset = 0.f;

void setSaveSlotOffset(int count)
{
    if ((!((saveSlotCursor-count) < 0 )) && (!((saveSlotCursor-count) > 4 )))
        return;

    if ((saveSlotCursor-count) < 0 ) {
        saveSlotCursor -= (saveSlotCursor-count);
    } else {
        saveSlotCursor-=1;
    }

    saveSlotOffset = 34.f * ((saveSlotCursor) - 4);

    if (saveSlotOffset < 0)
        saveSlotOffset = 0.f;
}

float getSaveSlotOffset()
{
    return saveSlotOffset;
}

void setIndicatorSlotNum(int count)
{
    indicatorSlotNum = count;
}

int getIndicatorSlotNum()
{
    return indicatorSlotNum;
}

void setActiveRectMap(rectMap_e rectmap)
{
    if(rectmap != DISPLAY_LAST) {
        ctr_rectMap.main = rectmap;
    }

    if(ctr_rectMap.fullscreen)
        ctr_rectMap.active = DISPLAY_FULL;
    else
        ctr_rectMap.active = ctr_rectMap.main;
}

void setPreviousRectMap(int index)
{
    if(index >= 0 && index < 2) {
        ctr_rectMap.prev[index] = ctr_rectMap.active;
    }
}

rectMap_e getPreviousRectMap(int index)
{
    if(index >= 0 && index < 2) {
        return ctr_rectMap.prev[index];
    }
    GNWSystemError("getPreviousRectMap failed\n");
    return DISPLAY_FULL;
}

void setRectMap(rectMap_e rectmap, int map_index, float src_x, float src_y, float src_w, float src_h,
        float dst_x, float dst_y, float dst_w, float dst_h)
{
    if (rectMaps[rectmap] == NULL) {
        rectMaps[rectmap] = (rectMap_t **)malloc(sizeof(rectMap_t *));
    } else {
        rectMaps[rectmap] = (rectMap_t **)realloc(rectMaps[rectmap], 
                (numRectsInMap[rectmap] + 1) * sizeof(rectMap_t *));
    }

    rectMaps[rectmap][numRectsInMap[rectmap]] = (rectMap_t *)malloc(sizeof(rectMap_t));
    if (rectMaps[rectmap][numRectsInMap[rectmap]] == NULL) {
        GNWSystemError("setRectMap failed\n");
        exit(EXIT_FAILURE);
    }

    rectMaps[rectmap][numRectsInMap[rectmap]]->src_x = src_x;
    rectMaps[rectmap][numRectsInMap[rectmap]]->src_y = src_y;
    rectMaps[rectmap][numRectsInMap[rectmap]]->src_w = src_w;
    rectMaps[rectmap][numRectsInMap[rectmap]]->src_h = src_h;

    rectMaps[rectmap][numRectsInMap[rectmap]]->dst_x = dst_x;
    rectMaps[rectmap][numRectsInMap[rectmap]]->dst_y = dst_y;
    rectMaps[rectmap][numRectsInMap[rectmap]]->dst_w = dst_w;
    rectMaps[rectmap][numRectsInMap[rectmap]]->dst_h = dst_h;

    numRectsInMap[rectmap]++;
}

void rectmap_init()
{
    if (rectMaps == NULL) {
        rectMaps = (rectMap_t ***)calloc(DISPLAY_LAST, sizeof(rectMap_t **));
        numRectsInMap = (int *)calloc(DISPLAY_LAST, sizeof(int));
        if (rectMaps == NULL || numRectsInMap == NULL) {
            GNWSystemError("rectmap_init failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

void ctr_rectmap_exit()
{
    if (rectMaps != NULL) {
        for (int i = 0; i < DISPLAY_LAST; ++i) {
            if (rectMaps[i] != NULL) {
                for (int j = 0; j < numRectsInMap[i]; ++j) {
                    free(rectMaps[i][j]);
                }
                free(rectMaps[i]);
            }
        }
        free(numRectsInMap);
        free(rectMaps);

        rectMaps = NULL;
        numRectsInMap  = NULL;
    }
}

void ctr_rectmap_init()
{
    rectmap_init();

    setRectMap(DISPLAY_FULL,              0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_FIELD,             0,   0,   0, 400, 300,   0,   0, 320, 240);
    setRectMap(DISPLAY_MOVIE,             0, 105,  80, 430, 320,  40,   0, 320, 240);
    setRectMap(DISPLAY_MAIN,              0, 400,  25, 215, 220,  43,   0, 234, 240);

    setRectMap(DISPLAY_GUI,               0,   5, 380, 200, 100,   0, 100, 200, 100); // monitor
    setRectMap(DISPLAY_GUI,               1, 520, 380, 140, 100, 200, 100, 140, 100); // skill pip
    setRectMap(DISPLAY_GUI,               2, 200, 380, 320, 100,   0,   0, 320, 100); // weapon

    setRectMap(DISPLAY_GUI_INDICATOR,     0,   0, 359, 127,  21,   0, 200, 127,  20); // stat indicator
    setRectMap(DISPLAY_GUI_INDICATOR,     1, 130, 359, 127,  21, 193, 200, 127,  20); // stat indicator
    setRectMap(DISPLAY_GUI_INDICATOR,     2, 260, 359, 127,  21,   0, 220, 127,  20); // stat indicator
    setRectMap(DISPLAY_GUI_INDICATOR,     3, 390, 359, 127,  21, 193, 220, 127,  20); // stat indicator

    setRectMap(DISPLAY_PAUSE,             0, 240,  72, 160, 215,  71,   0, 178, 240);
    setRectMap(DISPLAY_PAUSE_CONFIRM,     0, 170, 120, 300, 120,   0,  50, 320, 120);

    setRectMap(DISPLAY_DIALOG_TOP,        0, 130, 230, 390,  60,  10,   0, 380,  60); // dialog
    setRectMap(DISPLAY_DIALOG_TOP,        1,  80,   0, 480, 290,   0,   0, 397, 240);

    setRectMap(DISPLAY_DIALOG,            0, 125, 320, 345, 160,   0,  80, 320, 160); // bottom dialog
    setRectMap(DISPLAY_DIALOG,            1,   5, 410,  70,  70,  55,   0,  70,  70); // bottom review
    setRectMap(DISPLAY_DIALOG,            2, 565, 300,  70,  60, 125,   5,  70,  60); // bottom barter
    setRectMap(DISPLAY_DIALOG,            3, 565, 360,  70,  80, 195,   0,  70,  80); // bottom about
    setRectMap(DISPLAY_DIALOG_BACK,       0, 560, 440,  80,  36,   0,   0, 320,  80); // back

    setRectMap(DISPLAY_SKILLDEX,          0, 452,   5, 180, 190,   0,  40, 160, 190);
    setRectMap(DISPLAY_SKILLDEX,          1, 452, 190, 180, 180, 160,   0, 160, 180);

    setRectMap(DISPLAY_INVENTORY,         0,  95,  10, 470, 357,   0,   0, 320, 240);
    setRectMap(DISPLAY_INVENTORY_USE,     0,  80,   0, 290, 375,  52,   0, 215, 240); // single small inventory
    setRectMap(DISPLAY_INVENTORY_LOOT,    0,  95,  10, 505, 360,   0,   0, 320, 240);

    setRectMap(DISPLAY_INVENTORY_TRADE,   0,  88, 290, 460, 192,   0,  80, 320, 160); // during barter
    setRectMap(DISPLAY_INVENTORY_TRADE,   1,   0, 420,  80,  60,  80,   0,  80,  60); // offer
    setRectMap(DISPLAY_INVENTORY_TRADE,   2, 560, 420,  80,  60, 160,   0,  80,  60); // talk
    setRectMap(DISPLAY_INVENTORY_TRADE,   3,   0, 290,  80, 140,   0,   0,  60,  80); // during barter
    setRectMap(DISPLAY_INVENTORY_TRADE,   4, 560, 290,  80, 140, 260,   0,  60,  80); // during barter

    setRectMap(DISPLAY_INVENTORY_MOVE,    0, 140,  80, 258, 162,   0,  33, 320, 174);
    setRectMap(DISPLAY_INVENTORY_TIMER,   0, 140,  80, 258, 162,   0,  33, 320, 174);

    setRectMap(DISPLAY_AUTOMAP,           0,  60,   0, 520, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_WORLDMAP,          0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_PIPBOY,            0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_CHARACTER,         0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_VATS,              0, 110,  20, 420, 290,   0,   0, 320, 240);

    setRectMap(DISPLAY_LOADSAVE_TOP,      0, 337, 226, 280,  95, 155,   0, 230,  78); // desc
    setRectMap(DISPLAY_LOADSAVE_TOP,      1, 330,  30, 295, 190,  15,   0, 370, 238); // image

    setRectMap(DISPLAY_LOADSAVE_SLOT,     0,  50,  80, 220, 170,  32,  32, 270, 175); // slots

    setRectMap(DISPLAY_LOADSAVE,          0,  35,  58,  10,  27,   3, 131,  25,  70); // arrows

    setRectMap(DISPLAY_LOADSAVE,          1, 380, 344,  97,  26,  60,   0,  97,  26); // done
    setRectMap(DISPLAY_LOADSAVE,          2, 485, 344,  97,  26, 180,   0,  97,  26); // cancel

    setRectMap(DISPLAY_LOADSAVE_BACK,     0,  20,  20, 290, 200,   0,  40, 320, 200); // frame top
    setRectMap(DISPLAY_LOADSAVE_BACK,     1,  20, 440, 290,  20,   0,   0, 320,  40); // frame bottom

}

} //namespace fallout
