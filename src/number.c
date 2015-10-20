
#include "number.h"

void number_init(Number* n, u8 bits, u8 pos, u8 value)
{
    n->bits = bits;
    n->pos = pos;
    n->value = value;
}

void number_set_value(Number* n, u8 value)
{
    n->value = value;
}

void number_draw(Number* n, const u8* color)
{
    u8 index = n->pos;
    for (s8 i = n->bits - 1; i >= 0; i--)
    {
        u8 dimness = 5 * ((n->value & (1 << i)) == 0);
        plot_pad_dim(index, color, dimness);
        index++;
    }
}

u8 number_handle_press(Number* n, u8 index, u8 value)
{
    if (value > 0 && index >= n->pos && index < n->pos + n->bits)
    {
        n->value ^= 1 << (n->bits - (index - n->pos) - 1);
        return 1;
    }
    else
    {
        return 0;
    }
}
