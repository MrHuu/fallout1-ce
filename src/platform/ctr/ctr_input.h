#ifndef FALLOUT_PLATFORM_CTR_INPUT_H_
#define FALLOUT_PLATFORM_CTR_INPUT_H_

#include <3ds.h>

#include "plib/gnw/svga.h"

namespace fallout {

extern int offsetX;
extern int offsetY;

extern int offsetX_field;
extern int offsetY_field;

extern int currentInput;

enum active_input_t {
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
    active_input_t active;
    ctr_input_frame_t frame;
} ctr_input_t;

extern ctr_input_t ctr_input;

typedef struct
{
    int x;
    int y;
    unsigned rel_x;
    unsigned rel_y;
    int multiplier;
}qtm_state_t;
extern qtm_state_t qtm_state;

void ctr_input_get_touch(int *newX, int *newY);
void ctr_process_message();
void ctr_input_frame();

int ctr_input_swkbd(const char *hintText, const char *inText, char *outText);

void ctr_input_init();
void ctr_input_exit();

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_INPUT_H_ */
