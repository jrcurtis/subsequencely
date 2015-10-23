
#include "sequencer.h"


/*******************************************************************************
 * Sequencer functions
 ******************************************************************************/

void sequencer_init(Sequencer* sr)
{
    sr->swing_millis = 0;
    sr->step_millis = 100;
    sequencer_set_tempo(sr, DEFAULT_TEMPO);
    sr->timer = 0;
    sr->master_sequence = 0xFF;
    sr->active_sequence = 0;
    sr->soloed_tracks = 0;
    sr->flags = 0x00;

    scale_init(&sr->scale);

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        sequence_init(s, i, sr->note_bank + i * SEQUENCE_LENGTH);
        layout_init(&s->layout, &sr->scale, &sr->pad_notes, &sr->voices);
    }

    sequencer_set_active(sr, 0);
}

void sequencer_set_tempo_millis(Sequencer* sr, u16 millis)
{
    s8 swing = 6 * sr->swing_millis / sr->step_millis;

    sr->step_millis = millis;
    sr->clock_millis = sr->step_millis / TICKS_PER_STEP;

    sequencer_set_swing(sr, swing);
}

void sequencer_set_tempo(Sequencer* sr, u16 bpm)
{
    sequencer_set_tempo_millis(sr, bpm_to_millis(bpm));
}

void sequencer_set_swing(Sequencer* sr, s8 swing)
{
    sr->swing_millis = sr->step_millis * swing / 6;
}

void sequencer_set_octave(Sequencer* sr, u8 octave)
{
    Sequence* s = sequencer_get_active(sr);
    s->y = octave * s->layout.scale->num_notes;
}

void sequencer_set_active(Sequencer* sr, u8 i)
{
    sequence_become_inactive(sequencer_get_active(sr));

    sr->active_sequence = i;
    Sequence* s = &sr->sequences[i];
    sequence_become_active(s);

    keyboard_init(sr->keyboard, &s->layout);
    slider_set_value(sr->row_offset_slider,
                     s->layout.row_offset);
    checkbox_set_value(sr->control_checkbox,
                       flag_is_set(s->flags, SEQ_RECORD_CONTROL));
    number_set_value(sr->control_number,
                     s->control_code);
    slider_set_value(sr->control_sens_slider,
                     CC_SENS_RESOLUTION * GRID_SIZE
                     - s->control_div);
    slider_set_value(sr->control_offset_slider,
                     s->control_offset);

    voices_reset(&sr->voices);
}

Sequence* sequencer_get_active(Sequencer* sr)
{
    return &sr->sequences[sr->active_sequence];
}

void sequencer_find_master_sequence(Sequencer* sr)
{
    sr->master_sequence = 0xFF;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        if (flag_is_set(sr->sequences[i].flags, SEQ_PLAYING))
        {
            sr->master_sequence = i;
            break;
        }
    }
}

Layout* sequencer_get_layout(Sequencer* sr)
{
    return &sequencer_get_active(sr)->layout;
}

/*******************************************************************************
 * Drawing
 ******************************************************************************/

void sequencer_play_draw(Sequencer* sr)
{
    u8 play_index = LP_LAST_PLAY;
    const u8* color = off_color;
    u8 active_flags = 0;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];

        if (modifier_held(LP_RECORD_ARM))
        {
            color = flag_is_set(s->flags, SEQ_ARMED)
                ? number_colors[0]
                : off_color;
        }
        else if (modifier_held(LP_TRACK_SELECT))
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
                     || flag_is_set(s->flags, SEQ_QUEUED))
                ? sequence_colors[i]
                : off_color;
        }

        // If any track is armed AND playing, turn the record indicator on.
        if (flag_is_set(s->flags, SEQ_ARMED)
            && (flag_is_set(s->flags, SEQ_PLAYING)
                || flag_is_set(s->flags, SEQ_QUEUED)))
        {
            active_flags = set_flag(active_flags, SEQ_ARMED);
        }

        // If any tracks are muted/soloed, turn those indicators on.
        active_flags |= flag_is_set(s->flags, SEQ_MUTED) ? SEQ_MUTED : 0;
        active_flags |= flag_is_set(s->flags, SEQ_SOLOED) ? SEQ_SOLOED : 0;

        plot_pad(play_index, color);
        play_index -= LP_PLAY_GAP;
    }
    
    
    plot_pad(LP_RECORD_ARM,
        flag_is_set(active_flags, SEQ_ARMED)
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



/*******************************************************************************
 * Button handling
 ******************************************************************************/


u8 sequencer_handle_play(Sequencer* sr, u8 index, u8 value)
{
    if (value == 0
        || index < LP_FIRST_PLAY || index > LP_LAST_PLAY
        || (index - LP_FIRST_PLAY) % LP_PLAY_GAP != 0)
    {
        return 0;
    }

    // Figure out which sequence goes with which play button.
    // Play button indices get bigger going up, but tracks are layed out
    // with 0 at the top, so flip it around.
    u8 si = GRID_SIZE - 1 - (index - LP_FIRST_PLAY) / LP_PLAY_GAP;
    Sequence* s = &sr->sequences[si];

    if (modifier_held(LP_RECORD_ARM))
    {
        s->flags = toggle_flag(s->flags, SEQ_ARMED);
    }
    else if (modifier_held(LP_TRACK_SELECT))
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
        sr->soloed_tracks += flag_is_set(s->flags, SEQ_SOLOED) ? 1 : -1;

        // If this sequence has become soloed, and it is the only soloed
        // sequence, any other playing sequence must be killed.
        if (flag_is_set(s->flags, SEQ_SOLOED))
        {
            if (sr->soloed_tracks == 1)
            {
                for (u8 i = 0; i < GRID_SIZE; i++)
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
        else if (sr->soloed_tracks > 0)
        {
            sequence_kill_current_note(s);
        }
    }
    // If the sequence has been queued, or started playing stop it.
    // If we just stopped the master sequence, find a new one.
    else if (flag_is_set(s->flags, SEQ_QUEUED)
             || flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_stop(s);

        if (si == sr->master_sequence)
        {
            sequencer_find_master_sequence(sr);
        }
    }
    // Otherwise, queue it to start playing on the next step.
    // It becomes the master sequence if it's the least-numbered, playing
    // sequence.
    else
    {
        if (si < sr->master_sequence)
        {
            sr->master_sequence = si;
        }

        sequence_queue(s, modifier_held(LP_SHIFT));
    }

    return 1;
}

u8 sequencer_handle_record(Sequencer* sr)
{
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        u8 s_step_millis = s->clock_div * sr->step_millis;
        u8 quantize_ahead = (sr->timer % s_step_millis) > (s_step_millis / 4);
        sequence_handle_record(&sr->sequences[i], 1, quantize_ahead);
    }

    return 0;
}

/*******************************************************************************
 * Time handling
 ******************************************************************************/

void sequencer_tick(Sequencer* sr)
{
    sr->timer++;

    u16 channels = 0x0000;

    // If the timer is on an even clock interval, go through each
    // sequence and send clock for the playing ones. Keep track of which
    // channels have been seen so double messages aren't sent.
    if (sr->timer % sr->clock_millis == 0)
    {
        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            Sequence* s = &sr->sequences[i];
            u16 channel_flag = 1 << s->channel;

            if (!flag_is_set(channels, channel_flag)
                && flag_is_set(s->flags, SEQ_PLAYING))
            {
                channels = set_flag(channels, channel_flag);
                send_midi(MIDITIMINGCLOCK, 0x00, 0x00);
            }
        }
    }

    // step_millis is the number of milliseconds in a step, but if swing
    // is on, the even numbered steps are delayed and the odd ones are
    // rushed, so that it adds up to the same tempo over time.
    u16 step_millis = sr->step_millis;
    if (sr->master_sequence < GRID_SIZE)
    {
        step_millis += sr->sequences[sr->master_sequence].playhead % 2 == 0
            ? sr->swing_millis
            : -sr->swing_millis;
    }

    // If the timer hasn't passed the step threshold, return early,
    // but first check if we're halfway between steps, and give the sequence
    // a chance to turn off notes that would otherwise not have time to
    // fully turn off before the next note.
    // This uses sr->step_millis instead of the swing-adjusted step_millis
    // because otherwise a changing swing might cause it to be missed.
    if (sr->timer % sr->step_millis != 0)
    {
        if (sr->timer == sr->step_millis / 2)
        {
            for (u8 i = 0; i < GRID_SIZE; i++)
            {
                sequence_off_step(&sr->sequences[i]);
            }
        }

        return;
    }

    // Now we're ready to actually move forward 1 step. The timer is only reset
    // every 8 steps because it is used to implement clock division. The dirty
    // flag is used to force a redraw in states that are dependent on sequencer
    // state.
    u8 clock_step = sr->timer / sr->step_millis;
    if (clock_step >= GRID_SIZE)
    {
        sr->timer = 0;
    }
    sr->flags = set_flag(sr->flags, SQR_DIRTY);

    // Tick the master sequence first so that all subsequent sequences can
    // be told if they're on beat or not.
    u8 master_offset = (sr->master_sequence < GRID_SIZE)
        ? sr->master_sequence : 0;
    u8 on_beat = 1;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[(i + master_offset) % GRID_SIZE];

        u8 audible = !flag_is_set(s->flags, SEQ_MUTED)
            && (sr->soloed_tracks == 0
                || flag_is_set(s->flags, SEQ_SOLOED));

        if (s->clock_div == 1 || clock_step % s->clock_div == 0)
        {
            sequence_step(s, audible, on_beat);
        }

        if (i == 0)
        {
            on_beat = s->playhead % STEPS_PER_PAD == 0;
        }
    }
}
