
#include "number.h"

void number_draw(Number n, u8 pos, u8 bits, const u8* color)
{
    for (s8 i = bits - 1; i >= 0; i--)
    {
        u8 dimness = 5 * ((n & (1 << i)) == 0);
        plot_pad_dim(pos, color, dimness);
        pos++;
    }
}

u8 number_handle_press(Number* n, u8 index, u8 value, u8 pos, u8 bits)
{
    if (value > 0 && index >= pos && index < pos + bits)
    {
        *n ^= 1 << (bits - (index - pos) - 1);
        return 1;
    }
    else
    {
        return 0;
    }
}
