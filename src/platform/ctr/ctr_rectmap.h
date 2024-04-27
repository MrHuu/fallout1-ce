#ifndef FALLOUT_PLATFORM_CTR_RECTMAP_H
#define FALLOUT_PLATFORM_CTR_RECTMAP_H

namespace fallout {

typedef struct {
    float src_x, src_y, src_w, src_h;
    float dst_x, dst_y, dst_w, dst_h;
} rectMap_t;

extern rectMap_t ***rectMaps;
extern int *numRectsInMap;

typedef enum {
    DISPLAY_SPLASH = 0,
    DISPLAY_FULL,
    DISPLAY_FIELD,
    DISPLAY_GUI,
    DISPLAY_MOVIE,
    DISPLAY_SKILLDEX,
    DISPLAY_AUTOMAP,
    DISPLAY_WORLDMAP,
    DISPLAY_PIPBOY,
    DISPLAY_MAIN,
    DISPLAY_PAUSE,
    DISPLAY_PAUSE_CONFIRM,
    DISPLAY_DIALOG,
    DISPLAY_DIALOG_TOP,
    DISPLAY_DIALOG_BACK,
    DISPLAY_INVENTORY,
    DISPLAY_INVENTORY_USE,
    DISPLAY_INVENTORY_LOOT,
    DISPLAY_INVENTORY_TRADE,
    DISPLAY_INVENTORY_MOVE,
    DISPLAY_INVENTORY_TIMER,
    DISPLAY_CHARACTER,
    DISPLAY_VATS,
    DISPLAY_LAST
} rectMap_e;

struct ctr_rectMap_t {
    rectMap_e active;
    rectMap_e previous;
};
extern ctr_rectMap_t ctr_rectMap;

void setActiveRectMap(rectMap_e rectmap);
void setPreviousRectMap();

void ctr_rectmap_init();
void ctr_rectmap_exit();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_RECTMAP_H */