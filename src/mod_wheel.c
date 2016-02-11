
#include "mod_wheel.h"

int16_t mod_wheel_handle_press(ModWheel* m, uint8_t index, uint8_t value, uint8_t position)
{
    uint8_t pad = (index - position) / ROW_SIZE;

    if (pad >= MW_SIZE || (index - position) % ROW_SIZE != 0)
    {
        return 0;
    }

    uint8_t neg = 0;

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

void mod_wheel_draw(ModWheel m, uint8_t position)
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
        for (uint8_t i = 0; i < 2; i++)
        {
            uint8_t neg, neg_pad, pos_pad, value;
            const uint8_t* color;

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

            uint8_t dimness = value == 0 ? 3 : 4 - (value >> 5);
            plot_pad_dim(neg_pad, color, neg ? dimness : 3);
            plot_pad_dim(pos_pad, color, neg ? 3 : dimness);
        }
    }
}

uint16_t mod_wheel_get_value(ModWheel m)
{
    uint16_t value = MW_DEFAULT;
    int16_t diff = (m & MW_LSB_MASK) << (MW_BYTE_BITS - 2);
    diff |= 0x1F;

    if (flag_is_set(m, MW_LSB_NEG))
    {
        value -= diff;
    }
    else
    {
        value += diff;
    }

    diff = (m & MW_MSB_MASK) >> MW_BYTE_BITS;

    if (flag_is_set(m, MW_MSB_NEG))
    {
        value -= value * diff / 0x7F;
    }
    else
    {
        value += (MW_MAX - value) * diff / 0x7F;
    }

    return value;
}
