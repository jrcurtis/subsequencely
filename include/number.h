
#ifndef NUMBER_H
#define NUMBER_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef struct
{
    u8 bits;
    u8 pos;
    u8 value;
} Number;

void number_init(Number* n, u8 bits, u8 pos, u8 value);

void number_set_value(Number* n, u8 value);

void number_draw(Number* n, const u8* color);

u8 number_handle_press(Number* n, u8 index, u8 value);

#endif
