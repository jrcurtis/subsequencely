
#include "number.h"

void number_draw(Number n, uint8_t pos, uint8_t bits, const uint8_t* color)
{
    for (int8_t i = bits - 1; i >= 0; i--)
    {
        uint8_t dimness = 5 * ((n & (1 << i)) == 0);
        plot_pad_dim(pos, color, dimness);
        pos++;
    }
}

uint8_t number_handle_press(Number* n, uint8_t index, uint8_t value, uint8_t pos, uint8_t bits)
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
