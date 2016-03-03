
#include "scale.h"


void scale_init(Scale* s)
{
    s->num_notes = NUM_NOTES;
    s->notes = 0x0FFF;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        s->offsets[i] = i;
    }
}

void scale_update_offsets(Scale* s)
{
    uint8_t offset = 0;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        if (i < s->num_notes)
        {
            while (!scale_contains_note(s, offset))
            {
                offset++;
            }

            s->offsets[i] = offset;

            offset++;
        }
        else
        {
            s->offsets[i] = -1;
        }
    }
}

uint8_t scale_contains_note(Scale* s, uint8_t note)
{
    return flag_is_set(s->notes, 1 << note);
}

void scale_set_notes(Scale* s, uint16_t notes)
{
    s->notes = notes;
    s->num_notes = 0;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        s->num_notes += notes & 1;
        notes >>= 1;
    }

    scale_update_offsets(s);
}

void scale_toggle_note(Scale* s, uint8_t note)
{
    if (note < 1 || note >= NUM_NOTES)
    {
        return;
    }

    uint16_t flag = 1 << note;

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


