#ifndef FALLOUT_PLATFORM_CTR_RECTMAP_H
#define FALLOUT_PLATFORM_CTR_RECTMAP_H

namespace fallout {

typedef struct {
    float src_x, src_y, src_w, src_h;
    float dst_x, dst_y, dst_w, dst_h;
} rectMap_t;

extern rectMap_t ***rectMaps;
extern int *numRectsInMap;

enum active_display_mode_e {
    DISPLAY_MODE_ADAPT = 0,
    DISPLAY_MODE_FULL_BOT,
    DISPLAY_MODE_FULL_TOP,
    DISPLAY_MODE_LAST
};

typedef enum {
    DISPLAY_SPLASH = 0,
    DISPLAY_FULL,
    DISPLAY_FIELD,
    DISPLAY_GUI,
    DISPLAY_GUI_INDICATOR,
    DISPLAY_MOVIE,
    DISPLAY_MOVIE_SUB,
    DISPLAY_SKILLDEX,
    DISPLAY_WORLDMAP,
    DISPLAY_PIPBOY,
    DISPLAY_MAIN,
    DISPLAY_PAUSE,
    DISPLAY_DIALOG,
    DISPLAY_DIALOG_TOP,
    DISPLAY_DIALOG_BACK,
    DISPLAY_INVENTORY,
    DISPLAY_INVENTORY_USE,
    DISPLAY_INVENTORY_LOOT,
    DISPLAY_INVENTORY_TRADE,
    DISPLAY_CHAR_SELECT,
    DISPLAY_CHAR,
    DISPLAY_CHAR_EDIT_AGE,
    DISPLAY_CHAR_EDIT_SEX,
    DISPLAY_CHAR_TOP,
    DISPLAY_CHAR_PERK_TOP,
    DISPLAY_CHAR_PERK,
    DISPLAY_LOADSAVE_TOP,
    DISPLAY_LOADSAVE,
    DISPLAY_LOADSAVE_SLOT,
    DISPLAY_LOADSAVE_BACK,
    DISPLAY_ELEVATOR,
    DISPLAY_OPTIONS,
    DISPLAY_ENDGAME,
    DISPLAY_DYNAMIC,
    DISPLAY_DEAD,
    DISPLAY_LAST
} rectMap_e;

struct ctr_rectMap_t {
    rectMap_e main;
    rectMap_e active;
    rectMap_e prev[3];
};
extern ctr_rectMap_t ctr_rectMap;

extern bool isAgeWindow;
extern bool isSexWindow;
extern int offsetY_char;

float getSaveSlotOffset();
void setSaveSlotOffset(int count);

int getIndicatorSlotNum();
void setIndicatorSlotNum(int count);

void setRectMapPos(rectMap_e rectmap, float x, float y, float w, float h, bool isTop);

int setRectMapScaled(bool scaled_rect_top, float rawScaleFactor);

void setActiveRectMap(rectMap_e rectmap);
void setPreviousRectMap(int index);
rectMap_e getPreviousRectMap(int index);

void ctr_rectmap_init();
void ctr_rectmap_exit();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_RECTMAP_H */
