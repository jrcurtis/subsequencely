
#include "scale.h"


void scale_init(Scale* s)
{
    s->num_notes = NUM_NOTES;

    for (u8 i = 0; i < NUM_NOTES; i++)
    {
        s->notes[i] = 1;
        s->offsets[i] = i;
    }
}

void scale_update_offsets(Scale* s)
{
    u8 offset = 0;

    for (u8 i = 0; i < NUM_NOTES; i++)
    {
        while (!s->notes[offset])
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

void scale_toggle_note(Scale* s, u8 note)
{
    if (note < 1 || note >= NUM_NOTES)
    {
        return;
    }

    if (s->notes[note])
    {
        s->notes[note] = 0;
        s->num_notes--;
    }
    else
    {
        s->notes[note] = 1;
        s->num_notes++;
    }

    scale_update_offsets(s);
}


