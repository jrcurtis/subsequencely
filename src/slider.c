
#include "slider.h"

void slider_init(Slider* s, uint8_t resolution, int8_t offset, int8_t value)
{
    s->resolution = resolution;
    s->offset = offset;
    s->value = value - offset;
}

void slider_set_value(Slider* s, int8_t value)
{
    s->value = value - s->offset;
}

int8_t slider_get_value(Slider* s)
{
    return s->value + s->offset;
}

uint8_t slider_handle_press(Slider* s, uint8_t index, uint8_t value, uint8_t pos)
{
    uint8_t x, y;
    if (value == 0 || !index_to_pad(index, &x, &y) || y != pos)
    {
        return 0;
    }

    // All the pads before the one that was pressed are filled up, so find
    // their value, then find how much of the pressed pad is filled up based
    // on how hard it was pressed.
    s->value = x * s->resolution - 1;

    // The harder you press, the further away from 0 the value should be,
    // so if there's a negative offset and the pad that was pressed is on
    // the negative side of 0, then reverse the velocity.
    if (s->value + s->offset + s->resolution <= 0)
    {
        value = 127 - value;
    }

    // Each pad has a number of possible values, determined by s->resolution,
    // so figure out what fraction of them are filled and make sure at least
    // one is filled, so that the value doesn't creep into the range of the
    // previous pad.
    s->value += clamp(s->resolution * value / 100, 1, s->resolution);

    return 1;
}

void slider_draw(Slider* s, uint8_t pos, const uint8_t* color)
{
    uint8_t x = 0;

    // For positive offset (unipolar) sliders, all the pads before the value
    // are filled, but for negative offsets, only the pads between the one
    // that 0 is in and the one the value lies in are filled.
    uint8_t range_start = 0;
    uint8_t range_end = 0;
    uint8_t zero_point = max(0, -s->offset / s->resolution);
    uint8_t value_point = s->value / s->resolution;

    if (s->offset >= 0)
    {
        range_end = value_point;
    }
    else if (s->value + s->offset <= 0)
    {
        range_start = value_point;
        range_end = zero_point;
    }
    else 
    {
        range_start = zero_point;
        range_end = value_point;
    }

    int8_t prev_value = -1;

    for (x = 0; x < GRID_SIZE; x++)
    {
        const uint8_t* pad_color = off_color;
        if (x >= range_start
            && x <= range_end)
        {
            pad_color = color;
        }

        uint8_t dimness = 0;
        if (x == value_point)
        {
            // Figure out how much of the resolution of this pad is filled.
            // Reverse the direction for negative values just like in
            // handle_press.
            uint8_t pad_value = s->value - prev_value;
            if (s->value + s->offset <= 0)
            {
                pad_value = s->resolution - pad_value + 1;
            }

            // Fit the different possible resolutions into 5 brightness levels,
            // so that the lights can't go all the way off.
            dimness = 5 * (s->resolution - pad_value) / s->resolution;
        }

        plot_pad_dim(coord_to_index(x, pos), pad_color, dimness);

        prev_value += s->resolution;
    }
}
