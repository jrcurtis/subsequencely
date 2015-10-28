
#ifndef NUMBER_H
#define NUMBER_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef u8 Number;

/// Draws the individual bits of a number on screen.
void number_draw(Number n, u8 pos, u8 bits, const u8* color);

/// Toggles a pressed bit in a number.
u8 number_handle_press(Number* n, u8 index, u8 value, u8 pos, u8 bits);

#endif
