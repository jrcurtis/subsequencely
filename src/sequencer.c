
#include "sequencer.h"


/*******************************************************************************
 * Sequencer functions
 ******************************************************************************/

void sequencer_init(Sequencer* sr)
{
    sr->tempo = bpm_to_khz(DEFAULT_TEMPO);
    sr->timer = 0;
    sr->active_sequence = 0;
    sr->soloed_tracks = 0;
    sr->flags = 0x00;

    scale_init(&sr->scale);

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        sequence_init(s, i);
        layout_init(&s->layout, &sr->scale, &sr->pad_notes, &sr->voices);
    }

    sequencer_set_active(sr, 0);
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
                     8 - s->control_div);
    slider_set_value(sr->control_offset_slider,
                     s->control_offset);

    voices_reset(&sr->voices);
}

Sequence* sequencer_get_active(Sequencer* sr)
{
    return &sr->sequences[sr->active_sequence];
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
    // If the sequence has been queued, but hasn't started playing, just
    // unqueue it.
    else if (flag_is_set(s->flags, SEQ_QUEUED))
    {
        sequence_stop(s, &s->layout);
    }
    // If the sequence is playing, stop it and reset the playhead.
    else if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_stop(s, &s->layout);
    }
    // Otherwise, queue it to start playing on the next step.
    else
    {
        sequence_queue(s);
    }

    return 1;
}

u8 sequencer_handle_record(Sequencer* sr)
{
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        sequence_handle_record(&sr->sequences[i], 1);
    }

    return 0;
}

/*******************************************************************************
 * Time handling
 ******************************************************************************/

void sequencer_tick(Sequencer* sr)
{
    sr->timer++;

    if (sr->timer < sr->tempo)
    {
        if (sr->timer == sr->tempo / 2)
        {
            for (u8 i = 0; i < GRID_SIZE; i++)
            {
                sequence_off_step(&sr->sequences[i]);
            }
        }

        return;
    }

    sr->timer = 0;
    sr->flags = set_flag(sr->flags, SQR_DIRTY);

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        u8 audible = !flag_is_set(s->flags, SEQ_MUTED)
            && (sr->soloed_tracks == 0
                || flag_is_set(s->flags, SEQ_SOLOED));
        sequence_step(s, audible);
    }
}
