
#include "scale.h"


void scale_init(Scale* s)
{
    s->num_notes = NUM_NOTES;
    s->notes = 0x0000;

    for (u8 i = 0; i < NUM_NOTES; i++)
    {
        s->offsets[i] = i;
    }
}

void scale_update_offsets(Scale* s)
{
    u8 offset = 0;

    for (u8 i = 0; i < NUM_NOTES; i++)
    {
        while (!flag_is_set(s->notes, 1 << i))
        {
            offset++;
        }

        if (i < s->num_notes)
        {
            s->offsets[i] = offset;
        }
        else
        {
            s->offsets[i] = -1;
        }

        offset++;
    }
}

u8 scale_contains_note(Scale* s, u8 note)
{
    return flag_is_set(s->notes, 1 << note);
}

void scale_toggle_note(Scale* s, u8 note)
{
    if (note < 1 || note >= NUM_NOTES)
    {
        return;
    }

    u16 flag = 1 << note;

    if (flag_is_set(s->notes, flag))
    {
        s->notes = clear_flag(s->notes, flag);
        s->num_notes--;
    }
    else
    {
        s->notes = set_flag(s->notes, flag);
        s->num_notes++;
    }

    scale_update_offsets(s);
}


