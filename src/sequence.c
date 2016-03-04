
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

/*******************************************************************************
 * Sequence functions
 ******************************************************************************/

void sequence_init(Sequence* s, uint8_t channel, Note* notes)
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
    s->y = 3 * NUM_NOTES;
    s->zoom = 0;
    s->flags = 0x00;

    s->notes = notes;
    sequence_clear_notes(s);
}

Note* sequence_get_note(Sequence* s, uint8_t playhead)
{
    return &s->notes[playhead];
}

uint8_t sequence_get_channel(Sequence* s, uint8_t note_number)
{
    if (flag_is_set(s->flags, SEQ_DRUM_MULTICHANNEL))
    {
        return s->channel + note_number % NUM_NOTES;
    }
    else
    {
        return s->channel;
    }
}

uint8_t sequence_get_last_linked(Sequence* s)
{
    uint8_t linked_sequence = 1;

    while (flag_is_set(s[linked_sequence].flags, SEQ_LINKED_TO))
    {
        linked_sequence++;
    }

    return linked_sequence;
}

uint8_t sequence_get_next_playhead(Sequence* s)
{
    if (s->jump_step >= 0)
    {
        return s->jump_step;
    }

    int8_t direction = flag_is_set(s->flags, SEQ_REVERSED) ? -1 : 1;
    uint8_t next_playhead = s->playhead;
    uint8_t is_linked_to = flag_is_set(s->flags, SEQ_LINKED_TO);

    uint8_t last_seq_i = sequence_get_last_linked(s);

    // Check at most the number of notes in all the sequences linked to this
    // one, and if none of them are valid, just return the same playhead we're
    // already on.
    for (uint8_t i = 0; i < (last_seq_i + 1) * SEQUENCE_LENGTH; i++)
    {
        uint8_t linked_seq_i = next_playhead / SEQUENCE_LENGTH;
        uint8_t linked_playhead = next_playhead % SEQUENCE_LENGTH;
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
    sequence_prepare_mod_wheel(s);
}

void sequence_become_inactive(Sequence* s)
{
    s->flags = clear_flag(s->flags, SEQ_ACTIVE);
    layout_become_inactive(&s->layout);
}

void sequence_prepare_mod_wheel(Sequence* s)
{
    // When the mod wheel is visible, the notes are flipped negative so they
    // won't be mistaken for real notes when light_note is called.
    int8_t mul = flag_is_set(s->flags, SEQ_MOD_WHEEL) ? -1 : 1;
    for (uint8_t i = 0; i < MW_SIZE; i++)
    {
        lp_pad_notes[MOD_WHEEL_Y + i][MOD_WHEEL_X] =
            mul * abs(lp_pad_notes[MOD_WHEEL_Y + i][MOD_WHEEL_X]);
    }
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
                layout_light_note(&s->layout, n->note_number, 0);
            }
        }
        else
        {
            return;
        }
    }

    note_kill(n, sequence_get_channel(s, n->note_number));
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
           layout_light_note(&s->layout, n->note_number, 1);
        }
    }

    note_play(n, sequence_get_channel(s, n->note_number),
              flag_is_set(s->flags, SEQ_FULL_VELOCITY));
}

void sequence_play_current_note(Sequence* s)
{
    sequence_play_note(s, sequence_get_note(s, s->playhead));
}

void sequence_clear_note(Sequence* s, uint8_t step)
{
    Note* n = sequence_get_note(s, step);
    sequence_kill_note(s, n);
    note_init(n);
}

void sequence_clear_notes(Sequence* s)
{
    Sequence* linked_seq = s;
    uint8_t i = 0;
    while (1)
    {
        uint8_t seq_end = i + SEQUENCE_LENGTH;
        for (; i < seq_end; i++)
        {
            // clear_note must be called on original s so that noteoff is sent
            // on the right channel, if necessary.
            sequence_clear_note(s, i);
        }

        if (flag_is_set(linked_seq->flags, SEQ_LINKED_TO))
        {
            linked_seq++;
        }
        else
        {
            break;
        }
    }
}

void sequence_kill_voices(Sequence* s)
{
    for (uint8_t i = 0; i < voices_get_num_active(&lp_voices); i++)
    {
        uint8_t note_number = lp_voices.voices[i].note_number;
        uint8_t channel = sequence_get_channel(s, note_number);

        send_midi(NOTEOFF | channel, note_number, lp_voices.velocity);
    }

    voices_reset(&lp_voices);
}

void sequence_send_pitch_bend(Sequence* s)
{
    uint16_t bend = mod_wheel_get_value(lp_mod_wheel);
    send_midi(PITCHBEND | s->channel,
              bend & 0x7F,
              (bend >> 7) & 0x7F);
}

void sequence_send_aftertouch(Sequence *s, int8_t note_number, int8_t value)
{
    uint8_t channel = sequence_get_channel(s, note_number);

    if (note_number >= 0)
    {
        send_midi(
            POLYAFTERTOUCH | channel,
            note_number, value);
    }

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

    send_midi(CC | channel, s->control_code, value);
}

void sequence_transpose(Sequence* s, int8_t amt)
{
    sequence_kill_current_note(s);

    while (1)
    {
        for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
        {
            s->notes[i].note_number += amt;
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

void sequence_arpeggiate(Sequence* s, uint8_t pingpong)
{
    uint8_t i;
    uint8_t num_voices = voices_get_num_active(&lp_voices);

    for (i = 0; i < num_voices; i++)
    {
        Note* n = &s->notes[i];
        n->note_number = lp_voices.voices[i].note_number;
        n->velocity = 0x7F;
        n->flags = 0x00;
    }

    if (pingpong)
    {
        for (; i < 2 * num_voices - 2; i++)
        {
            Note* n = &s->notes[i];
            uint8_t pong_i = 2 * num_voices - i - 2;
            n->note_number = lp_voices.voices[pong_i].note_number;
            n->velocity = 0x7F;
            n->flags = 0x00;
        }
    }

    for (; i < SEQUENCE_LENGTH; i++)
    {
        Note* n = &s->notes[i];
        note_init(n);
        n->flags = NTE_SKIP;
    }
}

void sequence_set_skip(Sequence* s, uint8_t step, uint8_t skip)
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

Sequence* sequence_get_supersequence(Sequence* s)
{
    while (flag_is_set(s->flags, SEQ_LINKED))
    {
        s--;
    }

    return s;
}

void sequence_queue(Sequence* s, uint8_t queue_mode)
{
    sequence_queue_at(s, s->playhead, queue_mode);
}

void sequence_queue_at(Sequence* s, uint8_t step, uint8_t queue_mode)
{
    while (flag_is_set(s->flags, SEQ_LINKED))
    {
        s--;
        step += SEQUENCE_LENGTH;
    }

    sequence_kill_current_note(s);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    s->flags = seq_set_queued(s->flags, queue_mode);
    s->playhead = step;
}

void sequence_jump_to(Sequence* s, uint8_t step)
{
    while (flag_is_set(s->flags, SEQ_LINKED))
    {
        s--;
        step += SEQUENCE_LENGTH;
    }

    s->jump_step = step;
}

void sequence_queue_or_jump(Sequence* s, uint8_t step, uint8_t queue_mode)
{
    if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_jump_to(s, step);
    }
    else
    {
        sequence_queue_at(s, step, queue_mode);
    }
}

void sequence_stop(Sequence* s)
{
    s->flags = seq_set_queued(s->flags, 0);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    sequence_kill_current_note(s);
    s->playhead = 0;
    s->jump_step = -1;
}

void sequence_reverse(Sequence* s)
{
    s->flags = toggle_flag(s->flags, SEQ_REVERSED);
}

void sequence_handle_record(Sequence* s, uint8_t press)
{
    if (!flag_is_set(lp_flags, LP_ARMED)
        || !flag_is_set(s->flags, SEQ_ACTIVE)
        || !flag_is_set(s->flags, SEQ_PLAYING))
    {
        return;
    }

    uint16_t s_step_millis = s->clock_div * lp_sequencer.swung_step_millis;
    uint8_t quantize_ahead = lp_sequencer.step_timer > (s_step_millis / 4);
    Note* current_n = sequence_get_note(s, s->playhead);
    Note* record_n = quantize_ahead
        ? sequence_get_note(s, sequence_get_next_playhead(s))
        : current_n;
    int8_t played_note = voices_get_newest(&lp_voices);

    if (played_note > -1)
    {
        if (press)
        {
            sequence_kill_note(s, current_n);

            uint8_t slide = voices_get_num_active(&lp_voices) > 1;
            record_n->flags = assign_flag(record_n->flags, NTE_SLIDE, slide);

            s->flags = assign_flag(
                s->flags, SEQ_DID_RECORD_AHEAD, quantize_ahead);

            record_n->velocity = lp_voices.velocity;
        }
        else
        {
            current_n->flags = set_flag(current_n->flags, NTE_SLIDE);
            record_n->velocity = lp_voices.aftertouch;
        }

        record_n->note_number = played_note;
    }
}

void sequence_step(Sequence* s, uint8_t audible, uint8_t queue_flags)
{
    uint8_t queued = seq_get_queued(s->flags);

    // If this sequence is not playing and not about to start playing
    // skip it.
    if (!queued && !flag_is_set(s->flags, SEQ_PLAYING))
    {
        return;
    }
    // If it's about to start playing, check if it should become unqueued based
    // on the queue mode, and switch it to playing.
    else if (queued)
    {
        if (flag_is_set(queue_flags, 1 << queued))
        {
            s->flags = seq_set_queued(s->flags, 0);
            s->flags = set_flag(s->flags, SEQ_PLAYING);

            if (audible && !flag_is_set(s->flags, SEQ_DID_RECORD_AHEAD))
            {
                sequence_play_current_note(s);
            }
        }
    }
    // Otherwise find the next note, and if it is a slide note, play it
    // before killing the current one.
    else
    {
        uint8_t next_playhead = sequence_get_next_playhead(s);
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
        // If mod_cc is on, then control from the mod wheel takes precedence
        // over control recorded in the note the same way live notes take
        // precedence over sequence notes.
        if (audible
            && flag_is_set(s->flags, SEQ_RECORD_CONTROL)
            && flag_is_set(next_n->flags, NTE_ON)
            && !(flag_is_set(s->flags, SEQ_MOD_CC)
                 && lp_mod_wheel != 0))
        {
            note_control(next_n, s);
        }

        s->playhead = next_playhead;
    }
    
    // Handle the recording of held notes, but only if those held notes
    // weren't just recorded as pressed notes right before this.
    if (!flag_is_set(s->flags, SEQ_DID_RECORD_AHEAD))
    {
        sequence_handle_record(s, 0);
    }
    s->flags = clear_flag(s->flags, SEQ_DID_RECORD_AHEAD);
}

void sequence_off_step(Sequence* s)
{
    if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        uint8_t next_playhead = sequence_get_next_playhead(s);
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
 * Drawing
 ******************************************************************************/

void sequence_draw(Sequence* s)
{
    if (flag_is_set(s->flags, SEQ_MOD_WHEEL))
    {
        mod_wheel_draw(lp_mod_wheel, MOD_WHEEL_POS);
    }
}

/*******************************************************************************
 * Button handling
 ******************************************************************************/

uint8_t sequence_handle_press(Sequence* s, uint8_t index, uint8_t value)
{
    if (flag_is_set(s->flags, SEQ_MOD_WHEEL)
        && mod_wheel_handle_press(
            &lp_mod_wheel, index, value, MOD_WHEEL_POS))
    {
        if (flag_is_set(s->flags, SEQ_MOD_CC))
        {
            sequence_send_aftertouch(
                s, -1, mod_to_cc(lp_mod_wheel));
        }
        else
        {
            sequence_send_pitch_bend(s);
        }

        return 1;
    }

    int8_t note_number = layout_get_note_number(&s->layout, index);

    if (note_number > -1)
    {
        uint8_t channel = sequence_get_channel(s, note_number);
        uint8_t midi_message = 0;

        if (value > 0)
        {
            midi_message = NOTEON;
            voices_add(&lp_voices, note_number, value);
            sequence_handle_record(s, 1);
        }
        else if (!modifier_held(LP_SHIFT))
        {
            midi_message = NOTEOFF;
            voices_remove(&lp_voices, note_number);
        }

        if (midi_message != 0)
        {
            send_midi(
                midi_message | channel,
                note_number,
                flag_is_set(s->flags, SEQ_FULL_VELOCITY) ? 127 : value);
        }

        layout_light_note(&s->layout, note_number, value > 0);
    }
    else if (index == LP_SHIFT && value == 0)
    {
        sequence_kill_voices(s);
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
    else if (index == LP_RECORD)
    {
        sequence_arpeggiate(s, modifier_held(LP_SHIFT));
    }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t sequence_handle_aftertouch(Sequence* s, uint8_t index, int8_t value)
{
    int8_t note_number = -1;

    if (flag_is_set(s->flags, SEQ_MOD_WHEEL))
    {
        uint8_t handled = mod_wheel_handle_press(
            &lp_mod_wheel, index, value, MOD_WHEEL_POS);

        if (flag_is_set(s->flags, SEQ_MOD_CC))
        {
            if (handled)
            {
                value = mod_to_cc(lp_mod_wheel);
            }
            else
            {
                return 0;
            }
        }
        else if (handled)
        {
            sequence_send_pitch_bend(s);
            return 1;
        }
        else
        {
            note_number = layout_get_note_number(&s->layout, index);
        }
    }
    else
    {
        note_number = layout_get_note_number(&s->layout, index);
    }

    voices_handle_aftertouch(&lp_voices, note_number, value);

    if (flag_is_set(s->flags, SEQ_RECORD_CONTROL))
    {
        sequence_send_aftertouch(s, note_number, value);
    }

    return 1;
}
