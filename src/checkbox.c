
#include "checkbox.h"


void checkbox_init(Checkbox* c, u8 pos, u8 value)
{
    c->pos = pos;
    c->value = value;
}

void checkbox_set_value(Checkbox* c, u8 value)
{
    c->value = value != 0;
}

void checkbox_draw(Checkbox* c)
{
    plot_pad(c->pos, number_colors[c->value ? 2 : 0]);
}

u8 checkbox_handle_press(Checkbox* c, u8 index, u8 value)
{
    if (index == c->pos && value > 0)
    {
        c->value = !c->value;
        return 1;
    }
    else
    {
        return 0;
    }
}
