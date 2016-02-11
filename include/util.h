
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
#define TICKS_PER_BEAT   (24)
#define TICKS_PER_STEP   (TICKS_PER_BEAT / STEPS_PER_PAD)
#define SEQUENCE_LENGTH  (STEPS_PER_PAD * GRID_SIZE)
#define MAX_ZOOM         (2)
#define DEFAULT_TEMPO    (90)

#define min(a, b)       ((a) <= (b) ? (a) : (b))
#define max(a, b)       ((a) >= (b) ? (a) : (b))
#define clamp(x, a, b)  (min((b), max((a), (x))))
#define abs(a)          ((a) < 0 ? -(a) : (a))

// Converts between beats per minute and milliseconds per step.
#define bpm_to_millis(b)     (60000 / (b) / STEPS_PER_PAD)
#define millis_to_bpm(m)     (60000 / ((m) * STEPS_PER_PAD))

// Scales the given cc and adds the offset. div should be 1 or higher.
// sgn is 1 or -1. offset is 0-127.
#define cc_div(cc, sgn, div, off)  (clamp((sgn) * 4 * (cc) / (3 + (div)) + (off), 0, 127))

// Helpers for managing bitfields. These do not modify the passed in variable,
// an assignment is needed if you want to save this change.
#define flag_is_set(v, f)    (((v) & (f)) != 0)
#define set_flag(v, f)       ((v) | (f))
#define clear_flag(v, f)     ((v) & ~(f))
#define toggle_flag(v, f)    ((v) ^ (f))
#define assign_flag(v, f, b) ((b) ? set_flag((v), (f)) : clear_flag((v), (f)))

// Helpers for dealing with values that live in a subset of an int.
#define get_masked(v, m, o)    (((v) & (m)) >> (o))
#define set_masked(v, m, o, x) (((v) & ~(m)) | (((x) << (o)) & (m)))

// x, y coordinate with origin on lower left pad, to HAL index.
// only covers the pads, not the buttons around them.
#define coord_to_index(x, y)    (FIRST_PAD + ((ROW_SIZE) * (y)) + (x))

// Tries to convert the given index to coordinates and writes to the x and y
// parameters, or returns 0 if the index does not correspond to a pad.
uint8_t index_to_pad(uint8_t i, uint8_t* x, uint8_t* y);

// Row indices get bigger going up, but tracks are layed out with 0 at the top,
// so flip it around.
#define row_to_seq(y)           (GRID_SIZE - y - 1)

// Figure out which sequence goes with which play button. -1 on error.
#define index_to_play(i)        (((i) < LP_FIRST_PLAY                          \
                                 || (i) > LP_LAST_PLAY                         \
                                 || ((i) - LP_FIRST_PLAY) % LP_PLAY_GAP != 0)  \
                                 ? -1                                          \
                                 : row_to_seq(((i) - LP_FIRST_PLAY)            \
                                              / LP_PLAY_GAP))

// Shortcut to light an led with colors in a byte array.
#define plot_pad(i, c)          (hal_plot_led(                                 \
                                     TYPEPAD, (i),                             \
                                     (c)[0], (c)[1], (c)[2]))

// Shortcut to plot a pad, with colors dimmed by bitshifting the components.
// d shouldn't be 8 or more, or really any more than 5 or so, or else you'll
// shift all the bits right into oblivion.
#define plot_pad_dim(i, c, d)   (hal_plot_led(                                 \
                                     TYPEPAD, (i),                             \
                                     (c)[0] >> (d),                            \
                                     (c)[1] >> (d),                            \
                                     (c)[2] >> (d)))

// Plots the setup light at the botton of the launchpad.
#define plot_setup(c)           (hal_plot_led(                                 \
                                     TYPESETUP, 0,                             \
                                     (c)[0], (c)[1], (c)[2]))

// Convenience function that sends midi on the globally configured midi port.
#define send_midi(s, d1, d2)    (hal_send_midi(lp_midi_port, (s), (d1), (d2)))

// Handles tracking which modifier buttons are currently held.
extern uint32_t lp_modifiers;
#define modifier_held(m)        (flag_is_set(lp_modifiers, m##_FLAG))
void modifier_index_assign(uint8_t index, uint8_t value);

// Shortcuts for clearing all the leds at once.
void clear_leds();
void clear_pad_leds();

#ifdef VIRTUAL_LPP
#include <stdio.h>
#define LP_LOG(x, ...) printf(x "\n", __VA_ARGS__)
#else
#define LP_LOG(x, ...)
#endif

#endif
