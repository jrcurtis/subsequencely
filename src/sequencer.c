
#include "sequencer.h"

#define zoom_to_sequencer_x(sr)       (1 << (MAX_ZOOM - (sr)->zoom))

#define grid_to_sequencer_x(sr, gx)   (((gx) + (sr)->x)                        \
                                       * (1 << (MAX_ZOOM - (sr)->zoom)))

#define grid_to_sequencer_y(sr, gy)   ((sr)->layout->scale->offsets[           \
                                           ((gy) + (sr)->y)                    \
                                           % (sr)->layout->scale->num_notes    \
                                       ]                                       \
                                       + (sr)->layout->root_note               \
                                       + NUM_NOTES * (                         \
                                           ((gy) + (sr)->y)                    \
                                           / sr->layout->scale->num_notes      \
                                       ))

/*******************************************************************************
 * Sequence functions
 ******************************************************************************/

void sequence_init(Sequence* s, u8 channel)
{
    s->channel = channel;
    s->playhead = 0;
    s->flags = 0x00;

    for (u8 i = 0; i < SEQUENCE_LENGTH; i++)
    {
        s->notes[i].note_number = -1;
        s->notes[i].velocity = -1;
    }
}

void sequence_become_active(Sequence* s)
{
    s->flags = set_flag(s->flags, SEQ_ACTIVE);
}

void sequence_become_inactive(Sequence* s)
{
    s->flags = clear_flag(s->flags, SEQ_ACTIVE);

}

void sequence_kill_note(Sequence* s, Sequencer* sr)
{
    Note* n = &s->notes[s->playhead];
    if (n->note_number != -1)
    {
        if (flag_is_set(s->flags, SEQ_ACTIVE))
        {
            layout_light_note(sr->layout, n->note_number, n->velocity, 0);

            if (n->note_number == sr->layout->held_note)
            {
                return;
            }
        }

        hal_send_midi(
            USBSTANDALONE, NOTEOFF | s->channel,
            n->note_number, n->velocity);

    }
}

void sequence_play_note(Sequence* s, Sequencer* sr)
{
    Note* n = &s->notes[s->playhead];
    if (n->note_number != -1)
    {
        if (flag_is_set(s->flags, SEQ_ACTIVE))
        {
            layout_light_note(sr->layout, n->note_number, n->velocity, 1);
        }

        hal_send_midi(
            USBSTANDALONE, NOTEON | s->channel,
            n->note_number, n->velocity);
    }
}

void sequence_queue(Sequence* s)
{
    s->flags = set_flag(s->flags, SEQ_QUEUED);
}

void sequence_stop(Sequence* s, Sequencer* sr)
{
    s->flags = clear_flag(s->flags, SEQ_QUEUED);
    s->flags = clear_flag(s->flags, SEQ_PLAYING);
    sequence_kill_note(s, sr);
    s->playhead = 0;
}

void sequence_step(Sequence* s, Sequencer* sr)
{
    u8 enabled = !flag_is_set(s->flags, SEQ_MUTED)
        && (sr->soloed_tracks == 0
            || flag_is_set(s->flags, SEQ_SOLOED));

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
    }
    // If it's already playing, kill the current note and advance the
    // playhead.
    else
    {
        if (enabled)
        {
            sequence_kill_note(s, sr);
        }

        s->playhead = (s->playhead + 1) % SEQUENCE_LENGTH;
    }

    // Play the note.
    if (enabled)
    {
        sequence_play_note(s, sr);
    }
}

/*******************************************************************************
 * Sequencer functions
 ******************************************************************************/

void sequencer_init(Sequencer* sr, Layout* l)
{
    sr->layout = l;
    sr->tempo = bpm_to_khz(DEFAULT_TEMPO);
    sr->timer = 0;
    sr->zoom = 0;
    sr->active_sequence = 0;
    sr->soloed_tracks = 0;
    sr->x = 0;
    sr->y = 0;
    sr->flags = 0x00;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        sequence_init(&sr->sequences[i], i);
    }

    sequence_become_active(&sr->sequences[sr->active_sequence]);
}

void sequencer_set_octave(Sequencer* sr, u8 octave)
{
    sr->y = octave * sr->layout->scale->num_notes;
}

void sequencer_set_active(Sequencer* sr, u8 i)
{
    sequence_become_inactive(&sr->sequences[sr->active_sequence]);
    sr->active_sequence = i;
    sequence_become_active(&sr->sequences[i]);
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

        if (flag_is_set(sr->flags, SQR_ARM_HELD))
        {
            color = flag_is_set(s->flags, SEQ_ARMED)
                ? number_colors[0]
                : off_color;
        }
        else if (flag_is_set(sr->flags, SQR_SELECT_HELD))
        {
            color = sr->active_sequence == i
                ? number_colors[2]
                : off_color;
        }
        else if (flag_is_set(sr->flags, SQR_MUTE_HELD))
        {
            color = flag_is_set(s->flags, SEQ_MUTED)
                ? number_colors[1]
                : off_color;
        }
        else if (flag_is_set(sr->flags, SQR_SOLO_HELD))
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
        active_flags |= flag_is_set(s->flags, SEQ_MUTED);
        active_flags |= flag_is_set(s->flags, SEQ_SOLOED);

        plot_pad(play_index, color);
        play_index -= LP_PLAY_GAP;
    }
    
    
    plot_pad(LP_RECORD_ARM, flag_is_set(active_flags, SEQ_ARMED)
             ? number_colors[0] : off_color);
    plot_pad(LP_TRACK_SELECT, sequence_colors[sr->active_sequence]);
    plot_pad(LP_MUTE, flag_is_set(active_flags, SEQ_MUTED)
        ? number_colors[1] : off_color);
    plot_pad(LP_SOLO, flag_is_set(active_flags, SEQ_SOLOED)
             ? number_colors[3] : off_color);
}

void sequencer_grid_draw(Sequencer* sr)
{
    u8 scale_deg = sr->y % sr->layout->scale->num_notes;
    u8 octave = sr->y / sr->layout->scale->num_notes;
    Sequence* s = &sr->sequences[sr->active_sequence];
    u8 index = FIRST_PAD;
    u8 zoom = zoom_to_sequencer_x(sr);

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        u8 note_number = sr->layout->scale->offsets[scale_deg]
            + sr->layout->root_note
            + NUM_NOTES * octave;

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            u8 seq_x = grid_to_sequencer_x(sr, x);
            Note* n = &s->notes[seq_x];
            
            if (n->note_number == note_number)
            {
                const u8* color = number_colors[seq_x & 3];
                u8 dimness = 5 - n->velocity / (127 / 5);
                plot_pad_dim(index, color, dimness);
            }
            else if (s->playhead / zoom == x + sr->x)
            {
                plot_pad(index, on_color);
            }
            else if (layout_is_root_note(sr->layout, note_number))
            {
                plot_pad(index, root_note_color);
            }
            else
            {
                plot_pad(index, off_color);
            }

            index++;
        }

        scale_deg++;
        if (scale_deg >= sr->layout->scale->num_notes)
        {
            scale_deg = 0;
            octave++;
        }

        index += ROW_GAP;
    }
}


/*******************************************************************************
 * Button handling
 ******************************************************************************/

u8 sequencer_handle_translate(Sequencer* sr, u8 index, u8 value)
{
    if (shift_held || value == 0)
    {
        return 0;
    }

    if (index == LP_OCTAVE_UP)
    {
        if (grid_to_sequencer_y(sr, GRID_SIZE - 1) < MAX_NOTE)
        {
            sr->y++;
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (sr->y > 0)
        {
            sr->y--;
        }
    }
    else if (index == LP_TRANSPOSE_UP)
    {
        if ((sr->x + GRID_SIZE) * zoom_to_sequencer_x(sr) < SEQUENCE_LENGTH)
        {
            sr->x++;
        }
    }
    else if (index == LP_TRANSPOSE_DOWN)
    {
        if (sr->x > 0)
        {
            sr->x--;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequencer_handle_zoom(Sequencer* sr, u8 index, u8 value)
{
    if (!shift_held || value == 0)
    {
        return 0;
    }

    if (index == LP_OCTAVE_UP)
    {
        if (sr->zoom < MAX_ZOOM)
        {
            sr->zoom++;
            sr->x *= 2;
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (sr->zoom > 0)
        {
            sr->zoom--;
            sr->x /= 2;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

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

    if (flag_is_set(sr->flags, SQR_ARM_HELD))
    {
        s->flags = toggle_flag(s->flags, SEQ_ARMED);
    }
    else if (flag_is_set(sr->flags, SQR_SELECT_HELD))
    {
        sequencer_set_active(sr, si);
    }
    else if (flag_is_set(sr->flags, SQR_MUTE_HELD))
    {
        s->flags = toggle_flag(s->flags, SEQ_MUTED);

        // If the sequence has switched to muted, kill the current note.
        if (flag_is_set(s->flags, SEQ_MUTED))
        {
            sequence_kill_note(s, sr);
        }
    }
    else if (flag_is_set(sr->flags, SQR_SOLO_HELD))
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
                        sequence_kill_note(sk, sr);
                    }
                }
            }
        }
        // If this sequence is unsoloed, but other tracks are still soloed,
        // then this sequence essentially becomes muted, so kill it.
        else if (sr->soloed_tracks > 0)
        {
            sequence_kill_note(s, sr);
        }
    }
    // If the sequence has been queued, but hasn't started playing, just
    // unqueue it.
    else if (flag_is_set(s->flags, SEQ_QUEUED))
    {
        sequence_stop(s, sr);
    }
    // If the sequence is playing, stop it and reset the playhead.
    else if (flag_is_set(s->flags, SEQ_PLAYING))
    {
        sequence_stop(s, sr);
    }
    // Otherwise, queue it to start playing on the next step.
    else
    {
        sequence_queue(s);
    }

    return 1;
}

u8 sequencer_handle_modifiers(Sequencer* sr, u8 index, u8 value)
{
    if (index == LP_RECORD_ARM)
    {
        sr->flags = assign_flag(sr->flags, SQR_ARM_HELD, value > 0);
    }
    else if (index == LP_TRACK_SELECT)
    {
        sr->flags = assign_flag(sr->flags, SQR_SELECT_HELD, value > 0);
    }
    else if (index == LP_MUTE)
    {
        sr->flags = assign_flag(sr->flags, SQR_MUTE_HELD, value > 0);
    }
    else if (index == LP_SOLO)
    {
        sr->flags = assign_flag(sr->flags, SQR_SOLO_HELD, value > 0);
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequencer_grid_handle_press(Sequencer* sr, u8 index, u8 value)
{
    u8 x = 0;
    u8 y = 0;

    if (sequencer_handle_zoom(sr, index, value)) { }
    else if (sequencer_handle_translate(sr, index, value)) { }
    else if (index_to_pad(index, &x, &y))
    {
        u8 seq_x = grid_to_sequencer_x(sr, x);
        Sequence* s = &sr->sequences[sr->active_sequence];
        Note* n = &s->notes[seq_x];

        u8 note_number = grid_to_sequencer_y(sr, y);

        if (s->playhead == seq_x)
        {
            sequence_kill_note(s, sr);
        }

        if (value == 0)
        {

        }
        else if (n->note_number == note_number)
        {
            n->note_number = -1;
            n->velocity = -1;
        }
        else
        {
            n->note_number = note_number;
            n->velocity = value;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequencer_handle_record(Sequencer* sr)
{
    u8 note_recorded = 0;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &sr->sequences[i];
        Note* n = &s->notes[s->playhead];

        if (flag_is_set(s->flags, SEQ_ARMED)
            && flag_is_set(s->flags, SEQ_PLAYING)
            && sr->layout->held_note != -1)
        {
            sequence_kill_note(s, sr);
            n->note_number = sr->layout->held_note;
            n->velocity = sr->layout->held_velocity;
            note_recorded = 1;
        }
    }

    return note_recorded;
}

/*******************************************************************************
 * Time handling
 ******************************************************************************/

void sequencer_tick(Sequencer* sr)
{
    sr->timer++;

    if (sr->timer < sr->tempo)
    {
        return;
    }

    sr->timer = 0;

    // If the current sequence is playing when the sequence ticks, then a
    // display refresh is needed.
    if (flag_is_set(sr->sequences[sr->active_sequence].flags, SEQ_PLAYING))
    {
        sr->flags = set_flag(sr->flags, SQR_DIRTY);
    }

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        sequence_step(&sr->sequences[i], sr);
    }
}
