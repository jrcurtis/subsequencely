
#include "voices.h"

void voices_init(Voices* vs)
{
    vs->max_index = -1;
    vs->velocity = 0;
    vs->aftertouch = 0;

    for (u8 i = 0; i < NUM_VOICES; i++)
    {
        vs->voices[i].note_number = -1;
    }
}

void voices_add(Voices* vs, u8 note_number, u8 velocity)
{
    if (vs->max_index == NUM_VOICES - 1)
    {
        for (u8 i = 0; i < NUM_VOICES - 1; i++)
        {
            vs->voices[i] = vs->voices[i + 1];
        }
    }
    else
    {
        vs->max_index++;
    }

    vs->velocity = velocity;
    vs->voices[vs->max_index].note_number = note_number;
}

void voices_remove(Voices* vs, u8 note_number)
{
    u8 max_aftertouch = 0;
    u8 i;
    for (i = 0; i <= vs->max_index; i++)
    {
        if (vs->voices[i].note_number == note_number)
        {
            vs->max_index--;
            break;
        }
        else
        {
            max_aftertouch = max(max_aftertouch, vs->voices[i].aftertouch);
        }
    }

    for (; i <= vs->max_index; i++)
    {
        vs->voices[i] = vs->voices[i + 1];
        max_aftertouch = max(max_aftertouch, vs->voices[i].aftertouch);
    }

    vs->aftertouch = max_aftertouch;
}

u8 voices_handle_aftertouch(Voices* vs, u8 note_number, u8 aftertouch)
{
    u8 max_aftertouch = 0;

    for (u8 i = 0; i <= vs->max_index; i++)
    {
        if (vs->voices[i].note_number == note_number)
        {
            vs->voices[i].aftertouch = aftertouch;
        }

        max_aftertouch = max(max_aftertouch, vs->voices[i].aftertouch);
    }

    if (max_aftertouch != vs->aftertouch)
    {
        vs->aftertouch = max_aftertouch;
        return 1;
    }
    else
    {
        return 0;
    }
}

s8 voices_get_newest(Voices* vs)
{
    if (vs->max_index > -1)
    {
        return vs->voices[vs->max_index].note_number;
    }
    else
    {
        return -1;
    }
}

u8 voices_get_num_active(Voices* vs)
{
    return vs->max_index + 1;
}

void voices_reset(Voices* vs)
{
    vs->max_index = -1;
    vs->velocity = 0;
    vs->aftertouch = 0;
}
