#ifndef FALLOUT_PLATFORM_CTR_INPUT_H_
#define FALLOUT_PLATFORM_CTR_INPUT_H_

#include <3ds.h>

#include "plib/gnw/svga.h"

namespace fallout {

#define SINGLE_CLICK(x) kHeld & x && (!(oldpad & x))

#define MAX_OFFSET_X (640 - 400)
#define MAX_OFFSET_Y (480 - 240)

#define MAX_OFFSET_X_QTM 120
#define MAX_OFFSET_Y_QTM 120

extern int offsetX;
extern int offsetY;

extern int currentInput;

extern u32 kHeld;

typedef struct
{
    enum active_input_t { 
        INPUT_TOUCH = 0,
        INPUT_CPAD,
        INPUT_QTM,
        INPUT_LAST
    };
    active_input_t active;
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

void ctr_init_qtm();
void ctr_exit_qtm();
void ctr_input_frame();

int ctr_sys_swkbd(const char *hintText, const char *inText, char *outText);

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_INPUT_H_ */
