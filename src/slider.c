
#include "slider.h"

void slider_init(Slider* s, Orientation orientation, u8 position,
                 const u8* color, u8 resolution, s16 offset, s16 value)
{
    s->orientation = orientation;
    s->position = position;
    s->color = color;
    s->resolution = resolution;
    s->offset = offset;
    s->value = value - offset;
}

void slider_set_value(Slider* s, s16 value)
{
    s->value = value - s->offset;
}

s16 slider_get_value(Slider* s)
{
    return s->value + s->offset;
}

u8 slider_handle_press(Slider* s, u8 index, u8 value)
{
    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    u8 pad;

    if (s->orientation == VERTICAL && s->position == x)
    {
        pad = y;
    }
    else if (s->orientation == HORIZONTAL && s->position == y)
    {
        pad = x;
    }
    else
    {
        return 0;
    }

    s->value = pad * s->resolution;

    u8 bucket_size = 128 / s->resolution;
    s->value += (value + bucket_size) / bucket_size;

    return 1;
}

void slider_draw(Slider* s)
{
    u8 x = 0;
    u8 y = 0;
    u8* coord;

    if (s->orientation == VERTICAL)
    {
        x = s->position;
        coord = &y;
    }
    else
    {
        y = s->position;
        coord = &x;
    }

    u8 prev_value = 0;

    for (*coord = 0; *coord < GRID_SIZE; (*coord)++)
    {
        const u8* color = off_color;
        u8 dimness = 0;

        if (prev_value < s->value)
        {
            color = s->color;
        }

        u8 diff = s->value - prev_value;
        if (diff < s->resolution)
        {
            dimness = 5 * (s->resolution - diff) / s->resolution;
        }

        plot_pad_dim(coord_to_index(x, y), color, dimness);

        prev_value += s->resolution;
    }
}
