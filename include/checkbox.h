
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "app.h"
#include "colors.h"
#include "util.h"

typedef struct
{
    u8 pos;
    u8 value;
} Checkbox;

void checkbox_init(Checkbox* c, u8 pos, u8 value);

void checkbox_set_value(Checkbox* c, u8 value);

void checkbox_draw(Checkbox* c);

u8 checkbox_handle_press(Checkbox* c, u8 index, u8 value);

#endif
