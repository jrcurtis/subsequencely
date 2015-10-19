
#include "grid.h"

u8 grid_handle_translate(Sequencer* sr, u8 index, u8 value)
{
    if (shift_held || value == 0)
    {
        return 0;
    }

    Sequence* s = sequencer_get_active(sr);

    if (index == LP_OCTAVE_UP)
    {
        if (grid_to_sequence_y(s, GRID_SIZE - 1) < MAX_NOTE)
        {
            s->y++;
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (s->y > 0)
        {
            s->y--;
        }
    }
    else if (index == LP_TRANSPOSE_UP)
    {
        if ((s->x + GRID_SIZE) * zoom_to_sequence_x(s) < SEQUENCE_LENGTH)
        {
            s->x++;
        }
    }
    else if (index == LP_TRANSPOSE_DOWN)
    {
        if (s->x > 0)
        {
            s->x--;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 grid_handle_zoom(Sequencer* sr, u8 index, u8 value)
{
    if (!shift_held || value == 0)
    {
        return 0;
    }

    Sequence* s = sequencer_get_active(sr);

    if (index == LP_OCTAVE_UP)
    {
        if (s->zoom < MAX_ZOOM)
        {
            s->zoom++;
            s->x *= 2;
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (s->zoom > 0)
        {
            s->zoom--;
            s->x /= 2;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 grid_handle_press(Sequencer* sr, u8 index, u8 value)
{
    u8 x = 0;
    u8 y = 0;

    if (grid_handle_zoom(sr, index, value)) { }
    else if (grid_handle_translate(sr, index, value)) { }
    else if (index_to_pad(index, &x, &y))
    {
        Sequence* s = sequencer_get_active(sr);
        u8 seq_x = grid_to_sequence_x(s, x);
        Note* n = &s->notes[seq_x];

        u8 note_number = grid_to_sequence_y(s, y);

        if (s->playhead == seq_x)
        {
            sequence_kill_current_note(s);
        }

        if (value == 0)
        {

        }
        else if (n->note_number == note_number)
        {
            n->note_number = -1;
            n->velocity = 0;
            n->flags = 0x00;
        }
        else
        {
            n->note_number = note_number;
            n->velocity = value;
            n->flags = shift_held ? NTE_SLIDE : 0x00;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
