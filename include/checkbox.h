
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef u8 Checkbox;

#define CHECKBOX_ON_COLOR                    (number_colors[2])
#define CHECKBOX_OFF_COLOR                   (number_colors[0])
#define checkbox_draw(c, i)                  (plot_pad((i), ((c)                \
                                                       ? CHECKBOX_ON_COLOR     \
                                                       : CHECKBOX_OFF_COLOR)))

#define checkbox_handle_press(c, i, v, p)    (((i) == (p) && (v) > 0)          \
                                              ? (((c) = !(c)) || 1)            \
                                              : 0)

#endif
