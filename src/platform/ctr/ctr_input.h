#ifndef FALLOUT_PLATFORM_CTR_INPUT_H_
#define FALLOUT_PLATFORM_CTR_INPUT_H_

#include <3ds.h>

#include "plib/gnw/svga.h"

namespace fallout {

enum active_input_e {
    INPUT_TOUCH = 0,
    INPUT_CPAD,
    INPUT_LAST
};

typedef struct {
    int touchX;
    int touchY;
    u32 kHeld;
} ctr_input_frame_t;

typedef struct {
    u8 mode;
    active_input_e input;
    ctr_input_frame_t frame;
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

void ctr_input_center_mouse(void);

void ctr_input_get_touch(int *newX, int *newY);
void ctr_input_process(void);

int ctr_input_frame(void);

void ctr_input_swkdb_init(void);
int ctr_input_swkbd(const char *hintText, const char *inText, char *outText);

bool ctr_input_key_pressed(void);

void ctr_input_init(void);
void ctr_input_exit(void);

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_INPUT_H_ */
