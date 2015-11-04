
#include "mod_wheel.h"

int mod_wheel_handle_press(ModWheel* m, u8 index, u8 value, u8 position)
{
    u8 pad = (index - position) / ROW_SIZE;

    if (pad >= MW_SIZE || (index - position) % ROW_SIZE != 0)
    {
        return 0;
    }

    u8 neg = 0;

    switch (pad)
    {
        case 0:
            neg = value != 0;
        case 3:
            *m = (neg ? MW_MSB_NEG : 0)
                | (value << 7)
                | (*m & ~(MW_MSB_MASK | MW_MSB_NEG));
            break;

        case 1:
            neg = value != 0;
        case 2:
            *m = (neg ? MW_LSB_NEG : 0)
                | value
                | (*m & ~(MW_LSB_MASK | MW_LSB_NEG));
            break;
    }

    return 1;
}

void mod_wheel_draw(ModWheel m, u8 position)
{
    if (m == 0)
    {
        plot_pad_dim(position, drum_colors[2], 3);
        plot_pad_dim(position + ROW_SIZE, drum_colors[1], 3);
        plot_pad_dim(position + 2 * ROW_SIZE, drum_colors[1], 3);
        plot_pad_dim(position + 3 * ROW_SIZE, drum_colors[2], 3);
    }
    else
    {
        for (u8 i = 0; i < 2; i++)
        {
            u8 neg, neg_pad, pos_pad, value;
            const u8* color;

            if (i)
            {
                neg = flag_is_set(m, MW_LSB_NEG);
                neg_pad = position + ROW_SIZE;
                pos_pad = position + 2 * ROW_SIZE;
                value = m & MW_LSB_MASK;
                color = drum_colors[1];
            }
            else
            {
                neg = flag_is_set(m, MW_MSB_NEG);
                neg_pad = position;
                pos_pad = position + 3 * ROW_SIZE;
                value = (m & MW_MSB_MASK) >> 7;
                color = drum_colors[2];
            }

            u8 dimness = value == 0 ? 3 : 4 - (value >> 5);
            plot_pad_dim(neg_pad, color, neg ? dimness : 3);
            plot_pad_dim(pos_pad, color, neg ? 3 : dimness);
        }
    }
}

u16 mod_wheel_get_value(ModWheel m)
{
    u16 value = MW_DEFAULT;

    if (flag_is_set(m, MW_LSB_NEG))
    {
        value -= (m & MW_LSB_MASK) << 5;
    }
    else
    {
        value += (m & MW_LSB_MASK) << 5;
    }

    if (flag_is_set(m, MW_MSB_NEG))
    {
        value -= value * ((m & MW_MSB_MASK) >> 7) / 127;
    }
    else
    {
        value += (MW_MAX - value) * ((m & MW_MSB_MASK) >> 7) / 127;
    }

    return value;
}
