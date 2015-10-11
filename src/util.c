
#include "util.h"

#include "buttons.h"

u8 index_to_pad(u8 i, u8* x, u8* y)
{
    if (i < FIRST_PAD || i > LAST_PAD)
    {
        return 0;
    }

    u8 mod = i % (ROW_SIZE);

    if (mod == 0 || mod == ROW_SIZE - 1)
    {
        return 0;
    }

    *x = mod - 1;
    *y = i / ROW_SIZE - 1;

    return 1;
}

void clear_leds()
{
    for (u8 i = FIRST_BUTTON; i <= LAST_BUTTON; i++)
    {
        hal_plot_led(TYPEPAD, i, 0x00, 0x00, 0x00);
    }
}

void clear_pad_leds()
{
    u8 index = FIRST_PAD;

    for (u8 x = 0; index <= LAST_PAD; x++)
    {
        hal_plot_led(TYPEPAD, index, 0x00, 0x00, 0x00);

        if (x == GRID_SIZE)
        {
            x = 0;
            index += ROW_GAP;
        }
    }
}
