
#include "data.h"

#include "sequence.h"


/*******************************************************************************
 * Note functions
 ******************************************************************************/

void note_init(Note* n)
{
    n->note_number = -1;
    n->velocity = 0;
    n->flags = 0x00;
}

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
    if (n->velocity != -1)
    {
        send_midi(
            CC | s->channel,
            s->control_code,
            cc_div(n->velocity,
                   s->control_sgn,
                   s->control_div,
                   s->control_offset));
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

void sequence_init(Sequence* s, u8 channel, Note* notes)
{
    s->channel = channel;

    s->control_code = 0;
    s->control_div = 1;
    s->control_sgn = 1;
    s->control_offset = 0;

    s->playhead = 0;
    s->jump_step = -1;
    s->clock_div = 1;

    s->x = 0;
    s->y = 0;
    s->zoom = 0;
    s->flags = 0x00;

    s->notes = notes;
    sequence_clear_notes(s);
}

Note* sequence_get_note(Sequence* s, u8 playhead)
{
    return &s->notes[playhead];
}

u8 sequence_get_last_linked(Sequence* s)
{
    u8 linked_sequence = 1;

    while (flag_is_set(s[linked_sequence].flags, SEQ_LINKED_TO))
    {
        linked_sequence++;
    }

    return linked_sequence;
}

u8 sequence_get_next_playhead(Sequence* s)
{
    if (s->jump_step >= 0)
    {
        return s->jump_step;
    }

    s8 direction = flag_is_set(s->flags, SEQ_REVERSED) ? -1 : 1;
    u8 next_playhead = s->playhead;
    u8 is_linked_to = flag_is_set(s->flags, SEQ_LINKED_TO);

    u8 last_seq_i = sequence_get_last_linked(s);

    // Check at most the number of notes in all the sequences linked to this
    // one, and if none of them are valid, just return the same playhead we're
    // already on.
    for (u8 i = 0; i < (last_seq_i + 1) * SEQUENCE_LENGTH; i++)
    {
        u8 linked_seq_i = next_playhead / SEQUENCE_LENGTH;
        u8 linked_playhead = next_playhead % SEQUENCE_LENGTH;
        Sequence* linked_seq = &s[linked_seq_i];

        // If we're at the beginning, wrap around
        if (next_playhead == 0 && direction < 0)
        {
            // To the back of the most distant linked sequence
            if (is_linked_to)
            {
                next_playhead = last_seq_i * SEQUENCE_LENGTH
                    + SEQUENCE_LENGTH - 1;
            }
            // Or our own back
            else
            {
                next_playhead = SEQUENCE_LENGTH - 1;
            }
        }
        // If we're at the end of the current sequence either go to the next,
        // or wrap around.
        else if (linked_playhead == SEQUENCE_LENGTH - 1 && direction > 0)
        {
            if (flag_is_set(linked_seq->flags, SEQ_LINKED_TO))
            {
                next_playhead += 1;
            }
            else
            {
                next_playhead = 0;
            }
        }
        else
        {
            next_playhead += direction;
        }

        Note* n = sequence_get_note(s, next_playhead);
        if (!flag_is_set(n->flags, NTE_SKIP))
        {
            return next_playhead;
        }
    }

    return s->playhead;
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
            if (voices_get_newest(&lp_voices) == n->note_number)
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
    sequence_kill_note(s, sequence_get_note(s, s->playhead));
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
        if (voices_get_newest(&lp_voices) != -1)
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
    sequence_play_note(s, sequence_get_note(s, s->playhead));
}

void sequence_clear_note(Sequence* s, u8 step)
{
    Note* n = sequence_get_note(s, step);
    sequence_kill_note(s, n);
    note_init(n);
}

void sequence_clear_notes(Sequence* s)
{
    while (1)
    {
        for (u8 i = 0; i < SEQUENCE_LENGTH; i++)
        {
            sequence_clear_note(s, i);
        }

        if (flag_is_set(s->flags, SEQ_LINKED_TO))
        {
            s++;
        }
        else
        {
            break;
        }
    }
}

void sequence_set_skip(Sequence* s, u8 step, u8 skip)
{
    Note* n = sequence_get_note(s, step);
    n->flags = assign_flag(n->flags, NTE_SKIP, skip);
}

void sequence_toggle_linked_to(Sequence* s)
{
    s->flags = toggle_flag(s->flags, SEQ_LINKED_TO);
}

void sequence_toggle_linked(Sequence* s)
{
    sequence_stop(s);
    s->flags = toggle_flag(s->flags, SEQ_LINKED);
}

void sequence_queue(Sequence* s, u8 is_beat)
{
    sequence_queue_at(s, s->playhead, is_beat);
}

void sequence_queue_at(Sequence* s, u8 step, u8 is_beat)
{
    while (flag_is_set(s->flags, SEQ_LINKED))
    {
        s--;
        step += SEQUENCE_LENGTH;
    }

    sequence_kill_current_note(s);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    s->flags = assign_flag(s->flags, SEQ_BEAT_QUEUED, is_beat);
    s->flags = set_flag(s->flags, SEQ_QUEUED);
    s->playhead = step;
}

void sequence_jump_to(Sequence* s, u8 step)
{
    while (flag_is_set(s->flags, SEQ_LINKED))
    {
        s--;
        step += SEQUENCE_LENGTH;
    }

    s->jump_step = step;
}

void sequence_queue_or_jump(Sequence* s, u8 step, u8 is_beat)
{
    if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_jump_to(s, step);
    }
    else
    {
        sequence_queue_at(s, step, is_beat);
    }
}

void sequence_stop(Sequence* s)
{
    s->flags = clear_flag(s->flags, SEQ_QUEUED);
    s->flags = clear_flag(s->flags, SEQ_BEAT_QUEUED);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    sequence_kill_current_note(s);
    s->playhead = 0;
    s->jump_step = -1;
}

void sequence_reverse(Sequence* s)
{
    s->flags = toggle_flag(s->flags, SEQ_REVERSED);
}

void sequence_handle_record(Sequence* s, u8 press, u8 quantize_ahead)
{
    if (!flag_is_set(s->flags, SEQ_ARMED)
        || !flag_is_set(s->flags, SEQ_PLAYING))
    {
        return;
    }

    Note* current_n = sequence_get_note(s, s->playhead);
    Note* record_n = quantize_ahead
        ? sequence_get_note(s, sequence_get_next_playhead(s))
        : current_n;
    s8 played_note = voices_get_newest(&lp_voices);

    if (played_note > -1)
    {
        if (press)
        {
            sequence_kill_note(s, current_n);

            u8 slide = voices_get_num_active(&lp_voices) > 1;
            record_n->flags = assign_flag(record_n->flags, NTE_SLIDE, slide);

            s->flags = assign_flag(
                s->flags, SEQ_DID_RECORD_AHEAD, quantize_ahead);

            record_n->velocity = lp_voices.velocity;
        }
        else
        {
            current_n->flags = set_flag(current_n->flags, NTE_SLIDE);

            record_n->velocity = flag_is_set(s->flags, SEQ_RECORD_CONTROL)
                ? lp_voices.aftertouch
                : -1;
        }

        record_n->note_number = played_note;
    }
}

void sequence_step(Sequence* s, u8 audible, u8 is_beat)
{
    // If this sequence is not playing and not about to start playing
    // skip it.
    if (!flag_is_set(s->flags, SEQ_QUEUED)
        && !flag_is_set(s->flags, SEQ_PLAYING))
    {
        return;
    }
    // If it's about to start playing, switch it to playing.
    // If not on beat, only play if sequence isn't beat queued.
    else if (flag_is_set(s->flags, SEQ_QUEUED))
    {
        if (is_beat || !flag_is_set(s->flags, SEQ_BEAT_QUEUED))
        {
            s->flags = clear_flag(s->flags, SEQ_QUEUED);
            s->flags = clear_flag(s->flags, SEQ_BEAT_QUEUED);
            s->flags = set_flag(s->flags, SEQ_PLAYING);

            if (audible)
            {
                sequence_play_current_note(s);
            }
        }
    }
    // Otherwise find the next note, and if it is a slide note, play it
    // before killing the current one.
    else
    {
        u8 next_playhead = sequence_get_next_playhead(s);
        s->jump_step = -1;

        Note* n = sequence_get_note(s, s->playhead);
        Note* next_n = sequence_get_note(s, next_playhead);

        // If delete is held while the sequence plays, the playhead becomes an
        // erase head.
        if (lp_state == LP_NOTES_MODE
            && flag_is_set(s->flags, SEQ_ACTIVE)
            && modifier_held(LP_DELETE))
        {
            sequence_clear_note(s, s->playhead);
        }
        else if (!audible
                 || flag_is_set(s->flags, SEQ_DID_RECORD_AHEAD))
        {
            // Obviously if the sequence is muted or whatever don't play
            // anything, but also, if the last recorded note was quantized
            // a step ahead, then it was already heard when it was played
            // and doesn't need to be played again.
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
    
    // Handle the recording of held notes, but only if those held notes
    // weren't just recorded as pressed notes right before this.
    if (!flag_is_set(s->flags, SEQ_DID_RECORD_AHEAD))
    {
        sequence_handle_record(s, 0, 0);
    }
    s->flags = clear_flag(s->flags, SEQ_DID_RECORD_AHEAD);
}

void sequence_off_step(Sequence* s)
{
    if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        u8 next_playhead = sequence_get_next_playhead(s);
        Note* n = sequence_get_note(s, s->playhead);
        Note* next_n = sequence_get_note(s, next_playhead);

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
    if (layout_handle_press(&s->layout, index, value, s->channel))
    {
        return 1;
    }
    else if (value == 0)
    {
        return 0;
    }
    else if (index == LP_DELETE)
    {
        if (modifier_held(LP_SHIFT))
        {
            sequence_clear_notes(s);
        }
        else
        {
            sequence_clear_note(s, s->playhead);
        }
    }
    else if (index == LP_UNDO)
    {
        sequence_reverse(s);
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequence_handle_aftertouch(Sequence* s, u8 index, u8 value)
{
    if (flag_is_set(s->flags, SEQ_RECORD_CONTROL)
        && layout_handle_aftertouch(&s->layout, index, value, s))
    {

    }
    else
    {
        return 0;
    }

    return 1;
}
