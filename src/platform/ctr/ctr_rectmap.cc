#include <iostream>
#include <stdlib.h>

#include "ctr_rectmap.h"
#include "ctr_input.h"
#include "ctr_gfx.h"

#include "plib/gnw/gnw.h"
#include "game/loadsave.h"

namespace fallout {

#define MIN_SCALE_FACTOR 0.4167

#define MIN_SCALE_X 320
#define MIN_SCALE_Y 240

#define MAX_SCALE_X 640
#define MAX_SCALE_Y 480

#define SCALE_STEP 0.1

rectMap_t ***rectMaps = { NULL };
int *numRectsInMap = { NULL };

int indicatorSlotNum = 0;

int saveSlotCursor = 4;
float saveSlotOffset = 0.f;

static float scaleFactor = 1.f;

float offsetX_field = 160.f;
float offsetY_field = 40.f;

float offsetX_field_scaled_max;
float offsetY_field_scaled_max;

bool isAgeWindow = false;
bool isSexWindow = false;

int offsetY_char = 0;


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
    if (rectmap != DISPLAY_LAST) {
        ctr_rectMap.main = rectmap;
    }

    if (ctr_input.mode != DISPLAY_MODE_ADAPT)
        ctr_rectMap.active = DISPLAY_FULL;
    else
        ctr_rectMap.active = ctr_rectMap.main;

    if (ctr_rectMap.main == DISPLAY_GUI)
        ctr_input_center_mouse();
}

void setPreviousRectMap(int index)
{
    if (index >= 0 && index <= 2) {
        ctr_rectMap.prev[index] = ctr_rectMap.active;
    }
}

rectMap_e getPreviousRectMap(int index)
{
    if (index >= 0 && index <= 2) {
        return ctr_rectMap.prev[index];
    }
    GNWSystemError("getPreviousRectMap failed\n");
    return DISPLAY_FULL;
}

void setRectMapPos(rectMap_e rectmap, float x, float y, float w, float h, bool isTop)
{
    rectMaps[rectmap][0]->src_x = x;
    rectMaps[rectmap][0]->src_y = y;
    rectMaps[rectmap][0]->src_w = w;
    rectMaps[rectmap][0]->src_h = h;

    float maxDstWidth = isTop ? 400.f : 320.f;

    const float maxDstHeight = 240.f;

    float aspectRatio = w / h;

    float newWidth, newHeight;
    if (maxDstWidth / maxDstHeight > aspectRatio) {
        newHeight = maxDstHeight;
        newWidth = maxDstHeight * aspectRatio;
    } else {
        newWidth = maxDstWidth;
        newHeight = maxDstWidth / aspectRatio;
    }

    float newX = (maxDstWidth - newWidth) / 2.f;
    float newY = (maxDstHeight - newHeight) / 2.f;

    rectMaps[rectmap][0]->dst_x = newX;
    rectMaps[rectmap][0]->dst_y = newY;
    rectMaps[rectmap][0]->dst_w = newWidth;
    rectMaps[rectmap][0]->dst_h = newHeight;
}

int setRectMapScaled(bool scaled_rect_top, float rawScaleFactor)
{
    if (rawScaleFactor != -1.0) {
        float newScaleFactor = roundf(rawScaleFactor / SCALE_STEP) * SCALE_STEP;

        if (newScaleFactor < 0.0f) newScaleFactor = 0.0f;
        if (newScaleFactor > 1.0f) newScaleFactor = 1.0f;

        if (scaleFactor == newScaleFactor)
            return -1;

        scaleFactor = newScaleFactor;
    }

    float mappedScaleFactor = MIN_SCALE_FACTOR + (scaleFactor * (1.0f - MIN_SCALE_FACTOR));

    int scaled_x = roundf(MAX_SCALE_X - (MAX_SCALE_X - MIN_SCALE_X) * mappedScaleFactor);
    int scaled_y = roundf(MAX_SCALE_Y - (MAX_SCALE_Y - MIN_SCALE_Y) * mappedScaleFactor);

    if (scaled_rect_top)
        scaled_x += roundf(160 - (80 * mappedScaleFactor));

    rectMaps[DISPLAY_FIELD][0]->src_w = scaled_x;
    rectMaps[DISPLAY_FIELD][0]->src_h = scaled_y;

    offsetX_field_scaled_max = MAX_SCALE_X - scaled_x;
    offsetY_field_scaled_max = MAX_SCALE_Y - scaled_y;

    if (offsetY_field_scaled_max >= 100)
        offsetY_field_scaled_max -= 100;

    if (offsetY_field < 0)
        offsetY_field = 0;
    if (offsetY_field > offsetY_field_scaled_max)
        offsetY_field = offsetY_field_scaled_max;

    if (offsetX_field < 0)
        offsetX_field = 0;
    if (offsetX_field > offsetX_field_scaled_max)
        offsetX_field = offsetX_field_scaled_max;

    rectMaps[DISPLAY_FIELD][0]->src_x = offsetX_field;
    rectMaps[DISPLAY_FIELD][0]->src_y = offsetY_field;

    setRectMapPos(DISPLAY_FIELD,
            rectMaps[DISPLAY_FIELD][0]->src_x, rectMaps[DISPLAY_FIELD][0]->src_y,
            rectMaps[DISPLAY_FIELD][0]->src_w, rectMaps[DISPLAY_FIELD][0]->src_h,
            scaled_rect_top);

    return 0;
}

void addRectMap(rectMap_e rectmap, int map_index, float src_x, float src_y, float src_w, float src_h,
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
        GNWSystemError("addRectMap failed\n");
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
        numRectsInMap = NULL;
    }
}

void ctr_rectmap_init()
{
    rectmap_init();

    addRectMap(DISPLAY_FULL,              0,   0,   0, 640, 480,   0,   0, 320, 240);
    addRectMap(DISPLAY_FIELD,             0,   0,   0, 400, 240,   0,   0, 400, 240);
    addRectMap(DISPLAY_MOVIE,             0, 105,  80, 430, 320,  40,   0, 320, 240);
    addRectMap(DISPLAY_MOVIE_SUB,         0, 105, 401, 430,  80,   0,  60, 320, 120);
    addRectMap(DISPLAY_MAIN,              0, 400,  25, 215, 230,  43,   0, 234, 240);

    addRectMap(DISPLAY_GUI,               0,   5, 380, 200, 100,   0, 100, 200, 100); // monitor
    addRectMap(DISPLAY_GUI,               1, 520, 380, 140, 100, 200, 100, 140, 100); // skill pip
    addRectMap(DISPLAY_GUI,               2, 200, 380, 320, 100,   0,   0, 320, 100); // weapon

    addRectMap(DISPLAY_GUI_INDICATOR,     0,   0, 359, 127,  21,   0, 200, 127,  20); // stat indicator
    addRectMap(DISPLAY_GUI_INDICATOR,     1, 130, 359, 127,  21, 193, 200, 127,  20); // stat indicator
    addRectMap(DISPLAY_GUI_INDICATOR,     2, 260, 359, 127,  21,   0, 220, 127,  20); // stat indicator
    addRectMap(DISPLAY_GUI_INDICATOR,     3, 390, 359, 127,  21, 193, 220, 127,  20); // stat indicator

    addRectMap(DISPLAY_PAUSE,             0, 240,  72, 160, 215,  71,   0, 178, 240);

    addRectMap(DISPLAY_DIALOG_TOP,        0,  80,   0, 480, 290,   0,   0, 397, 240);
    addRectMap(DISPLAY_DIALOG_TOP,        1, 130, 230, 390,  60,  10,   0, 380,  60); // dialog

    addRectMap(DISPLAY_DIALOG,            0, 145, 320, 345, 160,   0,  80, 320, 160); // bottom dialog
    addRectMap(DISPLAY_DIALOG,            1,   5, 440,  70,  37,  45,  10,  70,  37); // bottom review
    addRectMap(DISPLAY_DIALOG,            2, 575, 324,  50,  28, 135,  15,  50,  28); // bottom barter
    addRectMap(DISPLAY_DIALOG,            3, 575, 399,  50,  28, 210,  15,  50,  28); // bottom about
    addRectMap(DISPLAY_DIALOG,            4,   5, 417,  70,  18,  48,  52,  70,  18); // bottom review text
    addRectMap(DISPLAY_DIALOG,            5, 565, 305,  70,  18, 125,  52,  70,  18); // bottom barter text
    addRectMap(DISPLAY_DIALOG,            6, 565, 360,  70,  35, 200,  44,  70,  35); // bottom about text

    addRectMap(DISPLAY_SKILLDEX,          0, 465,  50, 165, 140,  10,  50, 150, 140);
    addRectMap(DISPLAY_SKILLDEX,          1, 465, 195, 165, 140, 160,  50, 150, 140);
    addRectMap(DISPLAY_SKILLDEX,          2, 497,  14, 90,   29, 105, 195, 110,  35); // skilldex
    addRectMap(DISPLAY_SKILLDEX,          3, 485, 339, 117,  24, 102,  10, 117,  24); // cancel

    addRectMap(DISPLAY_INVENTORY,         0,  95,  10, 470, 357,   0,   0, 320, 240);
    addRectMap(DISPLAY_INVENTORY_USE,     0,  80,   0, 290, 375,  52,   0, 215, 240); // single small inventory
    addRectMap(DISPLAY_INVENTORY_LOOT,    0,  95,  10, 505, 360,   0,   0, 320, 240);

    addRectMap(DISPLAY_INVENTORY_TRADE,   0,  88, 290, 460, 192,   0,  80, 320, 160); // during barter
    addRectMap(DISPLAY_INVENTORY_TRADE,   1,   0, 420,  80,  60,  80,   0,  80,  60); // offer
    addRectMap(DISPLAY_INVENTORY_TRADE,   2, 560, 420,  80,  60, 160,   0,  80,  60); // talk
    addRectMap(DISPLAY_INVENTORY_TRADE,   3,   0, 290,  80, 140,   0,   0,  60,  80); // during barter
    addRectMap(DISPLAY_INVENTORY_TRADE,   4, 560, 290,  80, 140, 260,   0,  60,  80); // during barter

    addRectMap(DISPLAY_WORLDMAP,          0,   0,   0, 640, 480,   0,   0, 320, 240);
    addRectMap(DISPLAY_PIPBOY,            0,   0,   0, 640, 480,   0,   0, 320, 240);

    addRectMap(DISPLAY_CHAR_SELECT,       0,   0,   0, 640, 480,   0,   0, 320, 240); // char selection at game start

    addRectMap(DISPLAY_CHAR_TOP,          0, 335, 257, 300, 190,   0,   0, 400, 240);

    addRectMap(DISPLAY_CHAR,              0,  10,  30, 320, 210,   0,   0, 320, 210); // left
    addRectMap(DISPLAY_CHAR,              1, 340,   0, 320, 240,   0,   0, 320, 240); // right
    addRectMap(DISPLAY_CHAR,              2,  10,   0, 320,  30,   0, 210, 320,  30); // name / age / sex
    addRectMap(DISPLAY_CHAR,              3, 336, 450, 300,  25,   0,   0, 320,  25); // options / done / cancel

    addRectMap(DISPLAY_CHAR_EDIT_AGE,     0, 155,   0, 140,  70, 145, 170, 140,  70);
    addRectMap(DISPLAY_CHAR_EDIT_SEX,     0, 195,   0, 140,  70, 180, 170, 140,  70);

    addRectMap(DISPLAY_CHAR_PERK_TOP,     0, 304, 110, 290, 180,   0,   0, 400, 240);
    addRectMap(DISPLAY_CHAR_PERK,         0,  44, 105, 247, 199,   0,   0, 320, 240);

    addRectMap(DISPLAY_LOADSAVE_TOP,      0, 330,  30, 295, 190,  15,   0, 370, 238); // image
    addRectMap(DISPLAY_LOADSAVE_TOP,      1, 337, 226, 280,  95, 155,   0, 230,  78); // desc

    addRectMap(DISPLAY_LOADSAVE_SLOT,     0,  50,  80, 220, 170,  32,  32, 270, 175); // slots

    addRectMap(DISPLAY_LOADSAVE,          0,  35,  58,  10,  27,   3, 131,  25,  70); // arrows
    addRectMap(DISPLAY_LOADSAVE,          1, 380, 344,  97,  26,  60,   0,  97,  26); // done
    addRectMap(DISPLAY_LOADSAVE,          2, 485, 344,  97,  26, 180,   0,  97,  26); // cancel

    addRectMap(DISPLAY_LOADSAVE_BACK,     0,  20,  20, 290, 200,   0,  40, 320, 200); // frame top
    addRectMap(DISPLAY_LOADSAVE_BACK,     1,  20, 440, 290,  20,   0,   0, 320,  40); // frame bottom

    addRectMap(DISPLAY_OPTIONS,           0,   0,   0, 640, 480,   0,   0, 320, 240); // preferences TODO

    addRectMap(DISPLAY_ENDGAME,           0,   0,   0, 640, 480,   0,   0, 320, 240); // TODO
    addRectMap(DISPLAY_DYNAMIC,           0,   0,   0,   0,   0,   0,   0,   0,   0); // TODO
    addRectMap(DISPLAY_DEAD,              0,   0,   0, 640, 480,   0,   0, 320, 240); // TODO
}

} //namespace fallout
