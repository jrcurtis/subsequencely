
#include "sequence.h"


/*******************************************************************************
 * Note functions
 ******************************************************************************/

void note_play(Note* n, u8 channel)
{
    if (!flag_is_set(n->flags, NTE_ON))
    {
        n->flags = set_flag(n->flags, NTE_ON);
        send_midi(
            NOTEON | channel,
            n->note_number, n->velocity);
    }
}

void note_control(Note* n, Sequence* s)
{
    if (n->aftertouch != -1)
    {
        send_midi(
            CC | s->channel,
            s->control_code,
            n->aftertouch / s->control_div + s->control_offset);
    }
}

void note_kill(Note* n, u8 channel)
{
    if (flag_is_set(n->flags, NTE_ON))
    {
        n->flags = clear_flag(n->flags, NTE_ON);
        send_midi(
            NOTEOFF | channel,
            n->note_number, n->velocity);
    }
}

/*******************************************************************************
 * Sequence functions
 ******************************************************************************/

void sequence_init(Sequence* s, u8 channel)
{
    s->channel = channel;
    s->control_code = 0;
    s->control_div = 1;
    s->control_offset = 0;
    s->playhead = 0;
    s->x = 0;
    s->y = 0;
    s->zoom = 0;
    s->flags = 0x00;

    for (u8 i = 0; i < SEQUENCE_LENGTH; i++)
    {
        s->notes[i].note_number = -1;
        s->notes[i].velocity = 0;
        s->notes[i].aftertouch = -1;
        s->notes[i].flags = 0x00;
    }
}

u8 sequence_get_next_playhead(Sequence* s)
{
    return (s->playhead + 1) % SEQUENCE_LENGTH;
}

void sequence_become_active(Sequence* s)
{
    s->flags = set_flag(s->flags, SEQ_ACTIVE);
    layout_become_active(&s->layout);
}

void sequence_become_inactive(Sequence* s)
{
    s->flags = clear_flag(s->flags, SEQ_ACTIVE);
    layout_become_inactive(&s->layout);
}

void sequence_kill_note(Sequence* s, Note* n)
{
    if (n->note_number == -1)
    {
        return;
    }
    // If this is the active sequence, live playing should preempt the
    // sequence and live-played notes shouldn't be killed by the sequence.
    else if (flag_is_set(s->flags, SEQ_ACTIVE))
    {
        if (flag_is_set(n->flags, NTE_ON))
        {
            // If the user live-plays the same note the sequencer has already
            // played, then don't send a note off, but do mark the note as being
            // off, as it is the layout's responsibility to send the off
            // message.
            if (voices_get_newest(s->layout.voices) == n->note_number)
            {
                n->flags = clear_flag(n->flags, NTE_ON);
                return;
            }
            else
            {
                layout_light_note(&s->layout, n->note_number, n->velocity, 0);
            }
        }
        else
        {
            return;
        }
    }

    note_kill(n, s->channel);
}

void sequence_kill_current_note(Sequence* s)
{
    sequence_kill_note(s, &s->notes[s->playhead]);
}

void sequence_play_note(Sequence* s, Note* n)
{
    if (n->note_number == -1)
    {
        return;
    }
    // If this is the active sequence, then any active live-played notes
    // override the sequence, so don't turn them on or light them up.
    else if (flag_is_set(s->flags, SEQ_ACTIVE))
    {
        if (voices_get_newest(s->layout.voices) != -1)
        {
            return;
        }
        else
        {
           layout_light_note(&s->layout, n->note_number, n->velocity, 1);
        }
    }

    note_play(n, s->channel);
}

void sequence_play_current_note(Sequence* s)
{
    sequence_play_note(s, &s->notes[s->playhead]);
}

void sequence_queue(Sequence* s)
{
    s->flags = set_flag(s->flags, SEQ_QUEUED);
}

void sequence_queue_at(Sequence* s, u8 step)
{
    if (step < SEQUENCE_LENGTH)
    {
        sequence_kill_current_note(s);
        s->playhead = step;
        s->flags = clear_flag(s->flags, SEQ_PLAYING);
        s->flags = set_flag(s->flags, SEQ_QUEUED);
    }
}

void sequence_stop(Sequence* s)
{
    s->flags = clear_flag(s->flags, SEQ_QUEUED);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    sequence_kill_current_note(s);
    s->playhead = 0;
}

void sequence_handle_record(Sequence* s, u8 press)
{
    if (flag_is_set(s->flags, SEQ_ARMED)
        && flag_is_set(s->flags, SEQ_PLAYING))
    {
        Note* n = &s->notes[s->playhead];
        s8 played_note = voices_get_newest(s->layout.voices);

        if (played_note > -1)
        {
            if (press)
            {
                sequence_kill_note(s, n);
                u8 slide = voices_get_num_active(s->layout.voices) > 1;
                n->flags = assign_flag(n->flags, NTE_SLIDE, slide);
            }
            else
            {
                n->flags = set_flag(n->flags, NTE_SLIDE);
            }

            n->aftertouch = flag_is_set(s->flags, SEQ_RECORD_CONTROL)
                ? s->layout.voices->aftertouch
                : -1;

            n->note_number = played_note;
            n->velocity = s->layout.voices->velocity;
        }
    }
}

void sequence_step(Sequence* s, u8 audible)
{
    // If this sequence is not playing and not about to start playing
    // skip it.
    if (!flag_is_set(s->flags, SEQ_QUEUED)
        && !flag_is_set(s->flags, SEQ_PLAYING))
    {
        return;
    }
    // If it's about to start playing, switch it to playing.
    else if (flag_is_set(s->flags, SEQ_QUEUED))
    {
        s->flags = clear_flag(s->flags, SEQ_QUEUED);
        s->flags = set_flag(s->flags, SEQ_PLAYING);
        if (audible)
        {
            sequence_play_current_note(s);
        }
    }
    // Otherwise find the next note, and if it is a slide note, play it
    // before killing the current one.
    else
    {
        u8 next_playhead = sequence_get_next_playhead(s);
        Note* n = &s->notes[s->playhead];
        Note* next_n = &s->notes[next_playhead];

        if (!audible)
        {
            // Play nothing
        }
        else if (flag_is_set(n->flags, NTE_ON))
        {
            if (flag_is_set(next_n->flags, NTE_SLIDE))
            {
                if (n->note_number == next_n->note_number)
                {
                    n->flags = clear_flag(n->flags, NTE_ON);
                    next_n->flags = set_flag(next_n->flags, NTE_ON);
                }
                else
                {
                    sequence_play_note(s, next_n);
                    sequence_kill_note(s, n);
                }
            }
            else
            {
                sequence_kill_note(s, n);
                sequence_play_note(s, next_n);
            }
        }
        else
        {
            sequence_play_note(s, next_n);
        }

        // CC is handled separately because it should be sent even if the
        // note is held and no note on is being sent.
        if (audible
            && flag_is_set(s->flags, SEQ_RECORD_CONTROL))
        {
            note_control(next_n, s);
        }

        s->playhead = next_playhead;
    }
    
    // Handle the recording of notes that are still held down.
    sequence_handle_record(s, 0);
}

void sequence_off_step(Sequence* s)
{
    if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        u8 next_playhead = sequence_get_next_playhead(s);
        Note* n = &s->notes[s->playhead];
        Note* next_n = &s->notes[next_playhead];

        if (flag_is_set(n->flags, NTE_ON)
            && !flag_is_set(next_n->flags, NTE_SLIDE))
        {
            sequence_kill_note(s, n);
        }
    }
}

/*******************************************************************************
 * Button handling
 ******************************************************************************/

u8 sequence_handle_press(Sequence* s, u8 index, u8 value)
{
    if (layout_handle_press(&s->layout, index, value, s->channel)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequence_handle_aftertouch(Sequence* s, u8 index, u8 value)
{
    if (flag_is_set(s->flags, SEQ_RECORD_CONTROL)
        && layout_handle_aftertouch(
            &s->layout, index, value, s))
    {

    }
    else
    {
        return 0;
    }

    return 1;
}
