
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef u8 Checkbox;

#define CHECKBOX_ON_COLOR         (number_colors[2])
#define CHECKBOX_OFF_COLOR        (number_colors[0])
#define checkbox_draw(c, b, p)    (plot_pad((p), (flag_is_set((c), (b))        \
                                       ? CHECKBOX_ON_COLOR                     \
                                       : CHECKBOX_OFF_COLOR)))

#define checkbox_handle_press(c, b, i, v, p)    (                              \
                                  ((i) == (p) && (v) > 0)                      \
                                      ? (((c) = toggle_flag((c), (b))) || 1)   \
                                      : 0)

#endif
