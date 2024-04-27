#include <iostream>
#include <stdlib.h>

#include "ctr_input.h"
#include "ctr_gfx.h"
#include "ctr_rectmap.h"

#include "plib/gnw/gnw.h"

namespace fallout {

rectMap_t ***rectMaps = { NULL };
int *numRectsInMap = { NULL };

void setActiveRectMap(rectMap_e rectmap)
{
    ctr_rectMap.previous = ctr_rectMap.active;
    ctr_rectMap.active = rectmap;
}

void setPreviousRectMap()
{
    ctr_rectMap.active = ctr_rectMap.previous;
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

void ctr_rectmap_exit() {
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
    setRectMap(DISPLAY_MAIN,              0, 400,  25, 215, 220,  55,   0, 215, 240);

    setRectMap(DISPLAY_GUI,               0,   5, 380, 200, 100,   0,  10, 200, 100); // monitor
    setRectMap(DISPLAY_GUI,               1, 200, 380, 320, 100,   0, 140, 320, 100); // weapon
    setRectMap(DISPLAY_GUI,               2, 520, 380, 140, 100, 200,  10, 140, 100); // skill pip

    setRectMap(DISPLAY_PAUSE,             0, 240,  72, 160, 215,  55,   0, 215, 240);
    setRectMap(DISPLAY_PAUSE_CONFIRM,     0, 170, 120, 300, 120,   0,  50, 320, 120);

    setRectMap(DISPLAY_DIALOG_TOP,        0, 130, 230, 390,  60,  10,   0, 380,  60); // dialog
    setRectMap(DISPLAY_DIALOG_TOP,        1,  80,   0, 480, 290,   0,   0, 397, 240);

    setRectMap(DISPLAY_DIALOG,            0, 125, 320, 345, 160,   0,  80, 320, 160); // bottom dialog
    setRectMap(DISPLAY_DIALOG,            1,   5, 410,  70,  70,  55,   0,  70,  70); // bottom review
    setRectMap(DISPLAY_DIALOG,            2, 565, 300,  70,  60, 125,   5,  70,  60); // bottom barter
    setRectMap(DISPLAY_DIALOG,            3, 565, 360,  70,  80, 195,   0,  70,  80); // bottom about
    setRectMap(DISPLAY_DIALOG_BACK,       4, 560, 440,  80,  36,   0,   0, 320,  80); // back

    setRectMap(DISPLAY_SKILLDEX,          0, 452,   5, 180, 190,   0,  40, 160, 190);
    setRectMap(DISPLAY_SKILLDEX,          1, 452, 190, 180, 180, 160,   0, 160, 180);

    setRectMap(DISPLAY_INVENTORY,         0, 95,   10, 470, 357,   0,   0, 320, 240);
    setRectMap(DISPLAY_INVENTORY_USE,     0, 100, 100, 500, 300,   0,   0, 320, 240);
    setRectMap(DISPLAY_INVENTORY_LOOT,    0,  95,  10, 505, 360,   0,   0, 320, 240);
    setRectMap(DISPLAY_INVENTORY_TRADE,   0,   0, 290, 640, 210,   0,   0, 320, 120);
    setRectMap(DISPLAY_INVENTORY_MOVE,    0, 250, 200, 100, 100,  50,  50, 320, 240);
    setRectMap(DISPLAY_INVENTORY_TIMER,   0, 250, 200, 100, 100,  50,  50, 320, 240);

    setRectMap(DISPLAY_AUTOMAP,           0,  40,   0, 560, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_WORLDMAP,          0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_PIPBOY,            0,   0,   0, 640, 480,   0,   0, 320, 240);
    setRectMap(DISPLAY_VATS,              0, 110,  20, 420, 290,   0,   0, 320, 240);
}

} //namespace fallout
