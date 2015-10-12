
#include "slider.h"

void slider_init(Slider* s, Orientation o, u8 p, const u8* c, u8 v)
{
    s->orientation = o;
    s->position = p;
    s->color = c;
    s->value = v;
}

u8 slider_handle_press(Slider* s, u8 index, u8 value)
{
    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    if (s->orientation == VERTICAL && s->position == x)
    {
        s->value = y;
    }
    else if (s->orientation == HORIZONTAL && s->position == y)
    {
        s->value = x;
    }
    else
    {
        return 0;
    }

    return 1;
}

void slider_draw(Slider* s)
{
    u8 x = 0;
    u8 y = 0;

    if (s->orientation == VERTICAL)
    {
        x = s->position;
    }
    else if (s->orientation == HORIZONTAL)
    {
        y = s->position;
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        const u8* color = i <= s->value ? s->color : off_color;

        if (s->orientation == VERTICAL)
        {
            plot_pad(coord_to_index(x, i), color);
        }
        else if (s->orientation == HORIZONTAL)
        {
            plot_pad(coord_to_index(i, y), color);
        }
    }
}
