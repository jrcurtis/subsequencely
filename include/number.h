
#ifndef NUMBER_H
#define NUMBER_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef uint8_t Number;

/// Draws the individual bits of a number on screen.
void number_draw(Number n, uint8_t pos, uint8_t bits, const uint8_t* color);

/// Toggles a pressed bit in a number.
uint8_t number_handle_press(Number* n, uint8_t index, uint8_t value, uint8_t pos, uint8_t bits);

#endif
