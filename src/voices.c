
#include "voices.h"

void voices_init(Voices* vs)
{
    vs->num_active = 0;
    vs->velocity = 0;
    vs->aftertouch = 0;

    for (uint8_t i = 0; i < NUM_VOICES; i++)
    {
        vs->voices[i].note_number = -1;
        vs->voices[i].aftertouch = 0;
    }
}

void voices_add(Voices* vs, uint8_t note_number, uint8_t velocity)
{
    if (vs->num_active == NUM_VOICES)
    {
        for (uint8_t i = 0; i < NUM_VOICES - 1; i++)
        {
            vs->voices[i] = vs->voices[i + 1];
        }
    }
    else
    {
        vs->num_active++;
    }

    vs->velocity = velocity;
    vs->voices[vs->num_active - 1].note_number = note_number;
    vs->voices[vs->num_active - 1].aftertouch = 0;
}

void voices_remove(Voices* vs, uint8_t note_number)
{
    int8_t max_aftertouch = 0;
    uint8_t removed = 0;
    for (uint8_t i = 0; i < vs->num_active; i++)
    {
        if (removed)
        {
            vs->voices[i - 1] = vs->voices[i];
        }
        else if (vs->voices[i].note_number == note_number)
        {
            removed = 1;
            continue;
        }

        if (abs(vs->voices[i].aftertouch) > abs(max_aftertouch))
        {
            max_aftertouch = vs->voices[i].aftertouch;
        }
    }

    vs->num_active -= removed;
    vs->aftertouch = max_aftertouch;
}

uint8_t voices_handle_aftertouch(Voices* vs, int8_t note_number, int8_t aftertouch)
{
    int8_t max_aftertouch = aftertouch;

    for (uint8_t i = 0; i < vs->num_active; i++)
    {
        if (vs->voices[i].note_number == note_number)
        {
            vs->voices[i].aftertouch = aftertouch;
        }

        if (abs(vs->voices[i].aftertouch) > abs(max_aftertouch))
        {
            max_aftertouch = vs->voices[i].aftertouch;
        }
    }

    vs->aftertouch = max_aftertouch;
    return 1;
}

int8_t voices_get_newest(Voices* vs)
{
    if (vs->num_active > 0)
    {
        return vs->voices[vs->num_active - 1].note_number;
    }
    else
    {
        return -1;
    }
}

uint8_t voices_get_num_active(Voices* vs)
{
    return vs->num_active;
}

void voices_reset(Voices* vs)
{
    vs->num_active = 0;
    vs->velocity = 0;
    vs->aftertouch = 0;
}
