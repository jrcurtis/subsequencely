
#ifndef UTIL_H
#define UTIL_H

#include "app.h"

/*******************************************************************************
 * Defines/helpers
 ******************************************************************************/

#define NUM_NOTES   (12)
#define MAX_NOTE    (0x7F)
#define NUM_OCTAVES (MAX_NOTE / NUM_NOTES)

#define GRID_SIZE (8)
#define ROW_GAP   (2)
#define ROW_SIZE  (GRID_SIZE + ROW_GAP)
#define FIRST_PAD (11)
#define LAST_PAD  (FIRST_PAD + GRID_SIZE * ROW_SIZE - (ROW_GAP + 1))

#define STEPS_PER_PAD    (4)
#define SEQUENCE_LENGTH  (STEPS_PER_PAD * GRID_SIZE)
#define MAX_ZOOM         (2)

#define min(a, b)       ((a) <= (b) ? (a) : (b))
#define max(a, b)       ((a) >= (b) ? (a) : (b))
#define clamp(x, a, b)  (min((b), max((a), (x))))
#define abs(a)          ((a) < 0 ? -(a) : (a))

#define bpm_to_khz(b)   (60000 / (b) / STEPS_PER_PAD)

#define flag_is_set(v, f)    ((v) & (f))
#define set_flag(v, f)       ((v) | (f))
#define clear_flag(v, f)     ((v) & ~(f))
#define toggle_flag(v, f)    ((v) ^ (f))
#define assign_flag(v, f, b) ((b) ? set_flag((v), (f)) : clear_flag((v), (f)))

#define coord_to_index(x, y)    (FIRST_PAD + ((ROW_SIZE) * (y)) + (x))

u8 index_to_pad(u8 i, u8* x, u8* y);

#define plot_pad(i, c)          (hal_plot_led(                                 \
                                     TYPEPAD, (i),                             \
                                     (c)[0], (c)[1], (c)[2]))

#define plot_pad_dim(i, c, d)   (hal_plot_led(                                 \
                                     TYPEPAD, (i),                             \
                                     (c)[0] >> (d),                            \
                                     (c)[1] >> (d),                            \
                                     (c)[2] >> (d)))

#define plot_setup(c)           (hal_plot_led(                                 \
                                     TYPESETUP, 0,                             \
                                     (c)[0], (c)[1], (c)[2]))


void clear_leds();
void clear_pad_leds();

extern u8 shift_held;

#endif
