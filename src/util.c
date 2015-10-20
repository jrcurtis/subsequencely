
#include "util.h"

#include "buttons.h"

u32 modifiers = 0x00000000;

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
    for (u8 i = LP_FIRST_BUTTON; i <= LP_LAST_BUTTON; i++)
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

void modifier_index_assign(u8 index, u8 value)
{
    u32 flag = 1;

    if (index <= LP_STOP_CLIP)
    {
        flag <<= index - 1;
    }
    else if (index >= LP_OCTAVE_UP)
    {
        flag <<= index - LP_OCTAVE_UP + LP_STOP_CLIP;
    }
    else if (index >= LP_RECORD
             && index <= LP_SHIFT
             && (index - LP_RECORD) % ROW_SIZE == 0)
    {
        flag <<= (index - LP_RECORD) / ROW_SIZE
            + LP_STOP_CLIP
            + (LP_TRANSPOSE_UP - LP_OCTAVE_UP + 1);
    }
    else
    {
        return;
    }

    modifiers = assign_flag(modifiers, flag, value);
}
