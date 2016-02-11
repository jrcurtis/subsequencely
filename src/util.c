
#include "util.h"

#include "buttons.h"

uint32_t lp_modifiers = 0x00000000;

uint8_t index_to_pad(uint8_t i, uint8_t* x, uint8_t* y)
{
    static uint8_t last_i = 0xFF;
    static uint8_t last_x = 0;
    static uint8_t last_y = 0;

    if (i == last_i)
    {
        *x = last_x;
        *y = last_y;
        return 1;
    }
    else if (i < FIRST_PAD || i > LAST_PAD)
    {
        return 0;
    }

    uint8_t mod = i % (ROW_SIZE);

    if (mod == 0 || mod == ROW_SIZE - 1)
    {
        return 0;
    }

    *x = mod - 1;
    *y = i / ROW_SIZE - 1;

    last_i = i;
    last_x = *x;
    last_y = *y;

    return 1;
}

void clear_leds()
{
    for (uint8_t i = LP_FIRST_BUTTON; i <= LP_LAST_BUTTON; i++)
    {
        hal_plot_led(TYPEPAD, i, 0x00, 0x00, 0x00);
    }
}

void clear_pad_leds()
{
    uint8_t index = FIRST_PAD;

    for (uint8_t i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        if (i > 0 && i % GRID_SIZE == 0)
        {
            index += ROW_GAP;
        }
        
        hal_plot_led(TYPEPAD, index, 0x00, 0x00, 0x00);

        index++;
    }
}

void modifier_index_assign(uint8_t index, uint8_t value)
{
    uint32_t flag = 1;

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

    lp_modifiers = assign_flag(lp_modifiers, flag, value);
}
