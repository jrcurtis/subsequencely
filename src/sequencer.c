
#include <string.h>

#include "data.h"
#include "grid.h"

#include "sequencer.h"


/*******************************************************************************
 * Sequencer functions
 ******************************************************************************/

void sequencer_init(Sequencer* sr)
{

    sr->step_millis = 100;
    sr->clock_millis = 0;
    sr->swing_millis = 0;
    sr->swung_step_millis = 100;
    sr->step_timer = 0;
    sr->step_counter = 0;
    sr->clock_timer = 0;
    
    sequencer_set_tempo(sr, DEFAULT_TEMPO);

    sr->master_sequence = 0xFF;
    sr->active_sequence = 0;
    sr->soloed_sequences = 0;
    sr->copied_sequence = SQR_COPY_MASK;

    scale_init(&lp_scale);

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        sequence_init(s, i, lp_note_bank + i * SEQUENCE_LENGTH);
        layout_init(&s->layout);
    }

    for (uint16_t i = 0; i < SEQUENCE_LENGTH * GRID_SIZE; i++)
    {
        note_init(&lp_note_storage[i]);
    }

    sequencer_set_active(sr, 0);
}

void sequencer_set_tempo_millis(Sequencer* sr, uint8_t millis)
{
    // Remember the swing in a tempo-independent format so
    // the millis can be recalculated after the tempo change.
    int8_t swing = 6 * sr->swing_millis / sr->step_millis;

    sr->step_millis = millis;
    sr->clock_millis = sr->step_millis / TICKS_PER_STEP;
    sr->swung_step_millis = sr->step_millis;

    sequencer_set_swing(sr, swing);
}

void sequencer_set_tempo(Sequencer* sr, uint8_t bpm)
{
    sequencer_set_tempo_millis(sr, bpm_to_millis(bpm));
}

void sequencer_set_swing_millis(Sequencer* sr, int8_t swing_millis)
{
    sr->swing_millis = swing_millis;
}

void sequencer_set_swing(Sequencer* sr, int8_t swing)
{
    sr->swing_millis = sr->step_millis * swing / 6;
}

void sequencer_set_active(Sequencer* sr, uint8_t i)
{
    sequence_become_inactive(sequencer_get_active(sr));

    sr->active_sequence = i;
    Sequence* s = &sr->sequences[i];
    sequence_become_active(s);

    keyboard_init(&lp_keyboard, &s->layout);
    slider_set_value(&lp_row_offset_slider,
                     s->layout.row_offset & ROW_OFFSET_MASK);
    slider_set_value(&lp_control_sens_slider,
                     CC_SENS_RESOLUTION * GRID_SIZE
                     - s->control_div);
    slider_set_value(&lp_control_offset_slider,
                     s->control_offset);

    voices_reset(&lp_voices);

    if (lp_state == LP_SEQUENCER_MODE)
    {
        grid_update_cache(sr, 0);
    }

    lp_mod_wheel = 0;
}

Sequence* sequencer_get_active(Sequencer* sr)
{
    return &sr->sequences[sr->active_sequence];
}

void sequencer_find_master_sequence(Sequencer* sr)
{
    sr->master_sequence = 0xFF;

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        if (flag_is_set(sr->sequences[i].flags, SEQ_PLAYING))
        {
            sr->master_sequence = i;
            break;
        }
    }
}

Sequence* sequence_get_master(Sequencer* sr)
{
    if (sr->master_sequence < GRID_SIZE)
    {
        return &sr->sequences[sr->master_sequence];
    }
    else
    {
        return NULL;
    }
}

Layout* sequencer_get_layout(Sequencer* sr)
{
    return &sequencer_get_active(sr)->layout;
}

void sequencer_kill_current_notes(Sequencer* sr)
{
    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        sequence_kill_current_note(&sr->sequences[i]);
    }
}

void sequencer_copy(Sequencer* sr, uint8_t i, uint8_t swap)
{
    sr->copied_sequence = (swap ? SQR_COPY_SWAP : 0x00)
        | (i & SQR_COPY_MASK);
}

void sequencer_paste(Sequencer* sr, uint8_t i)
{
    uint8_t copied_sequence = sr->copied_sequence & SQR_COPY_MASK;
    uint8_t swap = (sr->copied_sequence & SQR_COPY_SWAP) != 0;

    if (copied_sequence == SQR_COPY_MASK)
    {
        return;
    }

    Note* from = (copied_sequence < GRID_SIZE)
        ? lp_note_bank
        : lp_note_storage;
    from += (copied_sequence % GRID_SIZE) * SEQUENCE_LENGTH;

    Note* to = (i < GRID_SIZE)
        ? lp_note_bank
        : lp_note_storage;
    to += (i % GRID_SIZE) * SEQUENCE_LENGTH;

    Sequence* s;
    if (i < GRID_SIZE)
    {
        s = sequence_get_supersequence(&sr->sequences[i]);
        sequence_kill_current_note(s);
    }

    if (copied_sequence < GRID_SIZE && swap)
    {
        s = sequence_get_supersequence(&sr->sequences[copied_sequence]);
        sequence_kill_current_note(s);
    }


    if (swap)
    {
        Note temp;
        for (uint8_t note_i = 0; note_i < SEQUENCE_LENGTH; note_i++)
        {
            temp = from[note_i];
            from[note_i] = to[note_i];
            to[note_i] = temp;
        }
    }
    else
    {
        memcpy(to, from, sizeof(Note) * SEQUENCE_LENGTH);
    }
}

void sequencer_copy_or_paste(Sequencer* sr, uint8_t i)
{
    if ((sr->copied_sequence & SQR_COPY_MASK) == SQR_COPY_MASK)
    {
        sequencer_copy(sr, i, sr->copied_sequence & SQR_COPY_SWAP);
    }
    else
    {
        sequencer_paste(sr, i);
    }
}

/*******************************************************************************
 * Drawing
 ******************************************************************************/

void sequencer_play_draw(Sequencer* sr)
{
    uint8_t play_index = LP_LAST_PLAY;
    const uint8_t* color = off_color;
    uint8_t active_flags = 0;

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];

        if (modifier_held(LP_TRACK_SELECT))
        {
            color = sr->active_sequence == i
                ? number_colors[2]
                : off_color;
        }
        else if (modifier_held(LP_MUTE))
        {
            color = flag_is_set(s->flags, SEQ_MUTED)
                ? number_colors[1]
                : off_color;
        }
        else if (modifier_held(LP_SOLO))
        {
            color = flag_is_set(s->flags, SEQ_SOLOED)
                ? number_colors[3]
                : off_color;
        }
        else
        {
            color = (flag_is_set(s->flags, SEQ_PLAYING)
                     || (seq_get_queued(s->flags) != 0
                         && lp_sequencer.step_counter % STEPS_PER_PAD == 0))
                ? sequence_colors[i]
                : off_color;
        }

        // If any tracks are muted/soloed, turn those indicators on.
        active_flags |= flag_is_set(s->flags, SEQ_MUTED) ? SEQ_MUTED : 0;
        active_flags |= flag_is_set(s->flags, SEQ_SOLOED) ? SEQ_SOLOED : 0;

        plot_pad(play_index, color);
        play_index -= LP_PLAY_GAP;
    }
    
    
    plot_pad(LP_RECORD_ARM,
        flag_is_set(lp_flags, LP_ARMED)
             ? number_colors[0]
             : off_color);
    
    plot_pad(LP_TRACK_SELECT,
        sequence_colors[sr->active_sequence]);
    
    plot_pad(LP_MUTE,
        flag_is_set(active_flags, SEQ_MUTED)
            ? number_colors[1]
            : off_color);
    
    plot_pad(LP_SOLO,
        flag_is_set(active_flags, SEQ_SOLOED)
             ? number_colors[3]
             : off_color);

}

void sequencer_blink_draw(Sequencer* sr, uint8_t blink, uint8_t position)
{
    uint8_t step;
    if (sr->master_sequence < GRID_SIZE)
    {
        step = sr->sequences[sr->master_sequence].playhead;
    }
    else
    {
        step = sr->step_counter;
    }

    if (blink)
    {
        plot_pad(LP_CLICK,
                 step % STEPS_PER_PAD == 0
                 ? on_color
                 : off_color);
    }

    if (position && sr->master_sequence < GRID_SIZE)
    {
        step = (step % SEQUENCE_LENGTH) / STEPS_PER_PAD;

        for (uint8_t i = 0; i < GRID_SIZE; i++)
        {
            const uint8_t* color;

            if (step == i)
            {
                color = on_color;
            }
            else if (lp_state == i - 4)
            {
                color = number_colors[lp_state];
            }
            else
            {
                color = off_color;
            }

            plot_pad(LP_OCTAVE_UP + i, color);
        }
    }
}

void sequencer_blink_clear(Sequencer* sr, uint8_t blink, uint8_t position)
{
    if (blink)
    {
        plot_pad(LP_CLICK, off_color);
    }

    if (position)
    {
        for (uint8_t i = 0; i < GRID_SIZE; i++)
        {
            const uint8_t* color = off_color;

            if (lp_state == i - 4)
            {
                color = number_colors[lp_state];
            }

            plot_pad(LP_OCTAVE_UP + i, color);
        }
    }
}


/*******************************************************************************
 * Button handling
 ******************************************************************************/


uint8_t sequencer_handle_play(Sequencer* sr, uint8_t index, uint8_t value)
{
    if (index == LP_RECORD_ARM && value > 0)
    {
        lp_flags = toggle_flag(lp_flags, LP_ARMED);
        return 1;
    }

    int8_t si = index_to_play(index);
    if (value == 0 || si == -1)
    {
        return 0;
    }

    Sequence* s = &sr->sequences[si];

    if (modifier_held(LP_TRACK_SELECT))
    {
        sequencer_set_active(sr, si);
    }
    else if (modifier_held(LP_MUTE))
    {
        s->flags = toggle_flag(s->flags, SEQ_MUTED);

        // If the sequence has switched to muted, kill the current note.
        if (flag_is_set(s->flags, SEQ_MUTED))
        {
            sequence_kill_current_note(s);
        }
    }
    else if (modifier_held(LP_SOLO))
    {
        s->flags = toggle_flag(s->flags, SEQ_SOLOED);
        sr->soloed_sequences += flag_is_set(s->flags, SEQ_SOLOED) ? 1 : -1;

        // If this sequence has become soloed, and it is the only soloed
        // sequence, any other playing sequence must be killed.
        if (flag_is_set(s->flags, SEQ_SOLOED))
        {
            if (sr->soloed_sequences == 1)
            {
                for (uint8_t i = 0; i < GRID_SIZE; i++)
                {
                    Sequence* sk = &sr->sequences[i];
                    if (i != si
                        && flag_is_set(sk->flags, SEQ_PLAYING)
                        && !flag_is_set(sk->flags, SEQ_MUTED))
                    {
                        sequence_kill_current_note(sk);
                    }
                }
            }
        }
        // If this sequence is unsoloed, but other tracks are still soloed,
        // then this sequence essentially becomes muted, so kill it.
        else if (sr->soloed_sequences > 0)
        {
            sequence_kill_current_note(s);
        }
    }
    // If the sequence has been queued, or started playing stop it.
    // If we just stopped the master sequence, find a new one.
    else if (seq_get_queued(s->flags) != 0
             || flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_stop(s);

        if (si == sr->master_sequence)
        {
            sequencer_find_master_sequence(sr);
            sequencer_blink_clear(sr, 0, 1);
        }
    }
    // Otherwise, queue it to start playing on the next step.
    // It becomes the master sequence if it's the least-numbered, playing
    // sequence.
    else
    {
        if (sr->master_sequence == 0xFF)
        {
            sr->master_sequence = si;
        }

        uint8_t queue_mode = sr->master_sequence == si ? SEQ_QUEUED_STEP
            : modifier_held(LP_SHIFT) ? SEQ_QUEUED_START
            : SEQ_QUEUED_BEAT;
        sequence_queue(s, queue_mode);
    }

    return 1;
}

/*******************************************************************************
 * Time handling
 ******************************************************************************/

void sequencer_tick(Sequencer* sr, uint8_t clock_tick)
{
    // If we just received a clock tick, immediately jump to the next clock
    // increment.
    if (clock_tick)
    {
        sr->step_timer += sr->clock_millis - sr->clock_timer;
        sr->clock_timer = sr->clock_millis;
    }
    // If we didn't receive a clock tick, but we ARE being clocked externally,
    // don't go to the next clock tick until we actually receive a clock
    // message. Just wait.
    else if (flag_is_set(lp_flags, LP_RCV_CLOCK)
             && !clock_tick
             && sr->clock_timer >= sr->clock_millis - 1)
    {
        return;
    }
    // If we aren't being clocked externally, or we are, but we're in between
    // clock ticks, just proceed 1 millisecond at a time as normal.
    else
    {
        sr->step_timer++;
        sr->clock_timer++;
    }

    // If the timer is on a clock interval, and clock is enabled, send clock.
    if (sr->clock_timer >= sr->clock_millis)
    {
        sr->clock_timer = 0;
        if (flag_is_set(lp_flags, LP_SEND_CLOCK))
        {
            send_midi(MIDITIMINGCLOCK, 0x00, 0x00);
        }
    }

    // If the timer hasn't passed the step threshold, return early, but first
    // check if we're close to the next step (one clock tick away), and give the
    // sequence a chance to turn off notes that would otherwise not have time to
    // fully turn off before the next note.
    if (sr->step_timer < sr->swung_step_millis)
    {
        if (sr->step_timer == sr->swung_step_millis / 2)
        {
            for (uint8_t i = 0; i < GRID_SIZE; i++)
            {
                sequence_off_step(&sr->sequences[i]);
            }
        }

        return;
    }

    // Now we're ready to actually move forward 1 step. The timer is reset and
    // the clock counter is reset after 8 steps to implement clock division.
    // The dirty flag is used to force a redraw in states that are dependent
    // on sequencer state.
    sr->step_timer = 0;
    sr->clock_timer = 0;
    sr->step_counter++;
    if (sr->step_counter >= STEP_COUNTER_WRAP)
    {
        sr->step_counter = 0;
    }
    lp_flags = set_flag(lp_flags, LP_SQR_DIRTY);

    // Tick the master sequence first so that all subsequent sequences can
    // be told if they're on the start of a beat or not.
    uint8_t master_offset = (sr->master_sequence < GRID_SIZE)
        ? sr->master_sequence : 0;
    uint8_t queue_flags = 1 << SEQ_QUEUED_STEP;

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        uint8_t seq_i = (i + master_offset) % GRID_SIZE;
        Sequence* s = &sr->sequences[seq_i];

        uint8_t audible = !flag_is_set(s->flags, SEQ_MUTED)
            && (sr->soloed_sequences == 0
                || flag_is_set(s->flags, SEQ_SOLOED));

        if (s->clock_div == 1 || sr->step_counter % s->clock_div == 0)
        {
            sequence_step(s, audible, queue_flags);

            if (flag_is_set(s->flags, SEQ_PLAYING))
            {
                if (seq_i < sr->master_sequence)
                {
                    sr->master_sequence = seq_i;
                }

                if (i == 0 && s->playhead % STEPS_PER_PAD == 0)
                {
                    queue_flags |= 1 << SEQ_QUEUED_BEAT;
                }

                if (s->playhead == 0)
                {
                    queue_flags |= 1 << SEQ_QUEUED_START;
                }
            }
        }
    }
    
    // Get ready for the next step by calculating how much to delay or rush.
    sr->swung_step_millis = sr->step_millis
        + (sr->sequences[master_offset].playhead % 2 == 0
              ? sr->swing_millis
              : -sr->swing_millis);
}

