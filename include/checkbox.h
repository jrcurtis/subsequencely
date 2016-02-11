
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "app.h"
#include "colors.h"
#include "util.h"

/// A checkbox is a single bit in a bitfield that can be drawn as a red or green
/// pad, and pressed on to flip the bit.
typedef uint8_t Checkbox;

#define CHECKBOX_ON_COLOR         (number_colors[2])
#define CHECKBOX_OFF_COLOR        (number_colors[0])

/// c: bitfield where the checkbox lives
/// b: bit the checkbox represents (1 << x)
/// p: the position to draw the checkbox
#define checkbox_draw(c, b, p)    (plot_pad((p), (flag_is_set((c), (b))        \
                                       ? CHECKBOX_ON_COLOR                     \
                                       : CHECKBOX_OFF_COLOR)))

/// c: bitfield where the checkbox bit lives
/// b: bit the checkbox represents (1 << x)
/// i: index of button that was pressed
/// v: velocity of the press
/// p: position of the checkbox itself
#define checkbox_handle_press(c, b, i, v, p)    (                              \
                                  ((i) == (p) && (v) > 0)                      \
                                      ? (((c) = toggle_flag((c), (b))) || 1)   \
                                      : 0)

#endif
