
#include "data.h"
#include "sequence.h"
#include "sequencer.h"

#include "voices.h"

void voices_init(Voices* vs)
{
    vs->num_active = 0;
    vs->num_sustained = 0;
    vs->velocity = 0;
    vs->aftertouch = 0;

    for (uint8_t i = 0; i < NUM_VOICES; i++)
    {
        vs->voices[i].note_number = -1;
        vs->voices[i].velocity = 0;
        vs->voices[i].flags = 0x00;
    }
}

Note* voices_add(Voices* vs, uint8_t note_number, uint8_t velocity)
{
    if (vs->num_active == NUM_VOICES)
    {
        Sequence* s = sequencer_get_active(&lp_sequencer);
        note_kill(&vs->voices[0],
                  sequence_get_channel(s, vs->voices[0].note_number));

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
    vs->voices[vs->num_active - 1].velocity = velocity;
    vs->voices[vs->num_active - 1].flags = 0x00;
    return &vs->voices[vs->num_active - 1];
}

Note* voices_remove(Voices* vs, uint8_t note_number)
{
    int8_t max_aftertouch = 0;
    uint8_t removed = 0;
    Note removed_note;

    for (uint8_t i = 0; i < vs->num_active; i++)
    {
        if (removed)
        {
            vs->voices[i - 1] = vs->voices[i];
        }
        else if (vs->voices[i].note_number == note_number)
        {
            removed = 1;
            removed_note = vs->voices[i];
            continue;
        }

        if (abs(vs->voices[i].velocity) > abs(max_aftertouch))
        {
            max_aftertouch = vs->voices[i].velocity;
        }
    }

    vs->num_active -= removed;
    vs->aftertouch = max_aftertouch;
    vs->voices[NUM_VOICES - 1] = removed_note;
    return &vs->voices[NUM_VOICES - 1];
}

void voices_sustain(Voices* vs, uint8_t enable)
{
    if (enable)
    {
        vs->num_sustained = vs->num_active;
    }
    else
    {
        vs->num_sustained = 0;
    }
}

void voices_remove_sustained(Voices* vs)
{
    for (uint8_t i = 0; i < vs->num_active - vs->num_sustained; i++)
    {
        vs->voices[i] = vs->voices[i + vs->num_sustained];
    }

    vs->num_active -= vs->num_sustained;
    vs->num_sustained = 0;
}

uint8_t voices_handle_aftertouch(Voices* vs, int8_t note_number, int8_t aftertouch)
{
    int8_t max_aftertouch = aftertouch;

    for (uint8_t i = 0; i < vs->num_active; i++)
    {
        if (vs->voices[i].note_number == note_number)
        {
            vs->voices[i].velocity = aftertouch;
        }

        if (abs(vs->voices[i].velocity) > abs(max_aftertouch))
        {
            max_aftertouch = vs->voices[i].velocity;
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

uint8_t voices_get_num_sustained(Voices* vs)
{
    return vs->num_sustained;
}

uint8_t voices_is_sustained(Voices* vs, uint8_t note_number)
{
    for (uint8_t i = 0; i < vs->num_sustained; i++)
    {
        if (vs->voices[i].note_number == note_number)
        {
            return 1;
        }
    }

    return 0;
}

void voices_reset(Voices* vs)
{
    vs->num_active = 0;
    vs->num_sustained = 0;
    vs->velocity = 0;
    vs->aftertouch = 0;
}
