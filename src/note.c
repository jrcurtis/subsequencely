
#include "data.h"
#include "sequence.h"

#include "note.h"

/*******************************************************************************
 * Note functions
 ******************************************************************************/

void note_init(Note* n)
{
    n->note_number = -1;
    n->velocity = 0;
    n->flags = 0x00;
}

void note_play(Note* n, uint8_t channel, uint8_t full_velocity)
{
    // Set the nte_on flag so that we remember to turn it off. Make sure that
    // a 0 velocity note on is not sent, as it gets interpreted as a note off.
    if (!flag_is_set(n->flags, NTE_ON))
    {
        n->flags = set_flag(n->flags, NTE_ON);
        send_midi(
            NOTEON | channel,
            n->note_number,
            full_velocity ? 127 : max(1, n->velocity));
    }
}

void note_control(Note* n, Sequence* s)
{
    if (n->velocity != -1)
    {
        int8_t value = n->velocity;

        if (value < -s->control_offset)
        {
            value = -s->control_offset;
        }
        else if (value > 127 - s->control_offset)
        {
            value = 127 - s->control_offset;
        }

        value = cc_div(
            value,
            s->control_sgn,
            s->control_div,
            s->control_offset);

        send_midi(
            CC | sequence_get_channel(s, n->note_number),
            s->control_code,
            value);
    }
}

void note_kill(Note* n, uint8_t channel)
{
    if (flag_is_set(n->flags, NTE_ON))
    {
        n->flags = clear_flag(n->flags, NTE_ON);
        send_midi(
            NOTEOFF | channel,
            n->note_number, n->velocity);
    }
}
