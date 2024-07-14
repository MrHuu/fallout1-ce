#ifndef FALLOUT_PLATFORM_CTR_INPUT_H_
#define FALLOUT_PLATFORM_CTR_INPUT_H_

#include <3ds.h>

#include "plib/gnw/svga.h"

namespace fallout {

enum active_input_e {
    INPUT_TOUCH = 0,
    INPUT_CPAD,
    INPUT_QTM,
    INPUT_LAST
};

typedef struct
{
    int touchX;
    int touchY;
    u32 kHeld;
} ctr_input_frame_t;

typedef struct
{
    int x;
    int y;
    unsigned rel_x;
    unsigned rel_y;
    int multiplier;
    QTM_HeadTrackingInfo qtminfo;
    u32 qtm_x, qtm_y;
    bool qtm_usable;
} qtm_state_t;

typedef struct
{
    u8 mode;
    active_input_e input;
    ctr_input_frame_t frame;
    qtm_state_t qtm_state;
} ctr_input_t;
extern ctr_input_t ctr_input;

extern int offsetX;
extern int offsetY;

extern float offsetX_field;
extern float offsetY_field;

extern int offsetX_field_scaled;
extern int offsetY_field_scaled;

extern float offsetX_field_scaled_max;
extern float offsetY_field_scaled_max;

extern int currentInput;

void ctr_input_get_touch(int *newX, int *newY);
void ctr_input_process();

int ctr_input_frame();
int ctr_input_swkbd(const char *hintText, const char *inText, char *outText);

void ctr_input_init();
void ctr_input_exit();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_INPUT_H_ */
