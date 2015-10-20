
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
#define DEFAULT_TEMPO    (90)

#define min(a, b)       ((a) <= (b) ? (a) : (b))
#define max(a, b)       ((a) >= (b) ? (a) : (b))
#define clamp(x, a, b)  (min((b), max((a), (x))))
#define abs(a)          ((a) < 0 ? -(a) : (a))

#define bpm_to_khz(b)   (60000 / (b) / STEPS_PER_PAD)
#define khz_to_bpm(k)   (60000 / (k * STEPS_PER_PAD))

#define flag_is_set(v, f)    (((v) & (f)) != 0)
#define set_flag(v, f)       ((v) | (f))
#define clear_flag(v, f)     ((v) & ~(f))
#define toggle_flag(v, f)    ((v) ^ (f))
#define assign_flag(v, f, b) ((b) ? set_flag((v), (f)) : clear_flag((v), (f)))

#define coord_to_index(x, y)    (FIRST_PAD + ((ROW_SIZE) * (y)) + (x))

u8 index_to_pad(u8 i, u8* x, u8* y);

#define row_to_seq(y)           (GRID_SIZE - y - 1)

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

extern u8 midi_port;
#define send_midi(s, d1, d2)    (hal_send_midi(midi_port, (s), (d1), (d2)))

extern u32 modifiers;
#define modifier_held(m)        (flag_is_set(modifiers, m##_FLAG))
void modifier_index_assign(u8 index, u8 value);

void clear_leds();
void clear_pad_leds();

#ifdef VIRTUAL_LPP
#include <stdio.h>
#define LP_LOG(x, ...) printf(x "\n", __VA_ARGS__)
#else
#define LP_LOG(x, ...)
#endif

#endif
