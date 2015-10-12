
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

void sequence_init(Sequence* s)
{
    s->playhead = 0;
    s->flags = 0x00;

    for (u8 i = 0; i < SEQUENCE_LENGTH; i++)
    {
        s->notes[i].note_number = -1;
        s->notes[i].velocity = -1;
    }
}

void sequence_kill_note(Sequence* s, u8 channel)
{
    Note* n = &s->notes[s->playhead];
    if (n->note_number != -1)
    {
        hal_send_midi(
            USBSTANDALONE, NOTEOFF | channel,
            n->note_number, n->velocity);
    }
}

void sequencer_init(Sequencer* sr, Sequences* ss, Layout* l)
{
    sr->sequences = ss;
    sr->layout = l;
    sr->tempo = bpm_to_khz(90);
    sr->timer = 0;
    sr->zoom = 0;
    sr->active_sequence = 0;
    sr->soloed_tracks = 0;
    sr->x = 0;
    sr->y = 0;
    sr->flags = 0x00;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        sequence_init(&(*sr->sequences)[i]);
    }
}

void sequencer_set_octave(Sequencer* sr, u8 octave)
{
    sr->y = octave * sr->layout->scale->num_notes;
}

/*******************************************************************************
 * Drawing
 ******************************************************************************/

void sequencer_play_draw(Sequencer* sr)
{
    u8 play_index = LAST_PLAY;
    const u8* color = off_color;


    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &(*sr->sequences)[i];

        if (flag_is_set(sr->flags, ARM_HELD))
        {
            color = flag_is_set(s->flags, ARMED)
                ? number_colors[0]
                : off_color;
        }
        else if (flag_is_set(sr->flags, SELECT_HELD))
        {
            color = sr->active_sequence == i
                ? number_colors[2]
                : off_color;
        }
        else if (flag_is_set(sr->flags, MUTE_HELD))
        {
            color = flag_is_set(s->flags, MUTED)
                ? number_colors[0]
                : off_color;
        }
        else if (flag_is_set(sr->flags, SOLO_HELD))
        {
            color = flag_is_set(s->flags, SOLOED)
                ? number_colors[3]
                : off_color;
        }
        else
        {
            color = flag_is_set(s->flags, PLAYING)
                ? sequence_colors[i]
                : off_color;
        }

        plot_pad(play_index, color);

        play_index -= PLAY_GAP;
    }
}

void sequencer_grid_draw(Sequencer* sr)
{
    u8 scale_deg = sr->y % sr->layout->scale->num_notes;
    u8 octave = sr->y / sr->layout->scale->num_notes;
    Sequence* s = &(*sr->sequences)[sr->active_sequence];
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

void sequencer_handle_translate(Sequencer* sr, u8 index)
{
    if (shift_held)
    {
        return;
    }

    if (index == OCTAVE_UP)
    {
        if (grid_to_sequencer_y(sr, GRID_SIZE - 1) < MAX_NOTE)
        {
            sr->y++;
        }
    }
    else if (index == OCTAVE_DOWN)
    {
        if (sr->y > 0)
        {
            sr->y--;
        }
    }
    else if (index == TRANSPOSE_UP)
    {
        if ((sr->x + GRID_SIZE) * zoom_to_sequencer_x(sr) < SEQUENCE_LENGTH)
        {
            sr->x++;
        }
    }
    else if (index == TRANSPOSE_DOWN)
    {
        if (sr->x > 0)
        {
            sr->x--;
        }
    }
}

void sequencer_handle_zoom(Sequencer* sr, u8 index)
{
    if (!shift_held)
    {
        return;
    }

    if (index == OCTAVE_UP)
    {
        if (sr->zoom < MAX_ZOOM)
        {
            sr->zoom++;
            sr->x *= 2;
        }
    }
    else if (index == OCTAVE_DOWN)
    {
        if (sr->zoom > 0)
        {
            sr->zoom--;
            sr->x /= 2;
        }
    }
}

void sequencer_handle_play(Sequencer* sr, u8 index)
{
    if (index < FIRST_PLAY || index > LAST_PLAY
        || (index - FIRST_PLAY) % PLAY_GAP != 0)
    {
        return;
    }

    u8 si = GRID_SIZE - 1 - (index - FIRST_PLAY) / PLAY_GAP;
    Sequence* s = &(*sr->sequences)[si];

    if (flag_is_set(sr->flags, ARM_HELD))
    {
        s->flags = toggle_flag(s->flags, ARMED);
    }
    else if (flag_is_set(sr->flags, SELECT_HELD))
    {
        sr->active_sequence = si;
    }
    else if (flag_is_set(sr->flags, MUTE_HELD))
    {
        s->flags = toggle_flag(s->flags, MUTED);
    }
    else if (flag_is_set(sr->flags, SOLO_HELD))
    {
        s->flags = toggle_flag(s->flags, SOLOED);
        sr->soloed_tracks += flag_is_set(s->flags, SOLOED) ? 1 : -1;
    }
    else
    {
        if (flag_is_set(s->flags, PLAYING))
        {
            s->flags = clear_flag(s->flags, PLAYING);
            sequence_kill_note(s, si);
            s->playhead = 0;
        }
        else
        {
            s->flags = set_flag(s->flags, QUEUED);
        }
    }
}

void sequencer_grid_handle_press(Sequencer* sr, u8 index, u8 value)
{
    u8 x = 0;
    u8 y = 0;

    if (index == RECORD_ARM)
    {
        sr->flags = assign_flag(sr->flags, ARM_HELD, value > 0);
    }
    else if (index == TRACK_SELECT)
    {
        sr->flags = assign_flag(sr->flags, SELECT_HELD, value > 0);
    }
    else if (index == MUTE)
    {
        sr->flags = assign_flag(sr->flags, MUTE_HELD, value > 0);
    }
    else if (index == SOLO)
    {
        sr->flags = assign_flag(sr->flags, SOLO_HELD, value > 0);
    }
    else if (index_to_pad(index, &x, &y))
    {
        u8 seq_x = grid_to_sequencer_x(sr, x);
        Sequence* s = &(*sr->sequences)[sr->active_sequence];
        Note* n = &s->notes[seq_x];

        u8 note_number = grid_to_sequencer_y(sr, y);

        if (s->playhead == seq_x)
        {
            sequence_kill_note(s, sr->active_sequence);
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
    else if (value > 0)
    {
        sequencer_handle_play(sr, index);
        sequencer_handle_zoom(sr, index);
        sequencer_handle_translate(sr, index);
    }
}

/*******************************************************************************
 * Time handling
 ******************************************************************************/

void sequencer_tick(Sequencer* sr)
{
    sr->timer++;

    if (sr->timer >= sr->tempo)
    {
        sr->timer = 0;
        sr->flags = set_flag(sr->flags, DIRTY);

        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            Sequence* s = &(*sr->sequences)[i];
            Note* n = &s->notes[s->playhead];
            u8 enabled = !flag_is_set(s->flags, MUTED)
                && (sr->soloed_tracks == 0
                    || flag_is_set(s->flags, SOLOED));

            // If this sequence is not playing and not about to start playing
            // skip it.
            if (!flag_is_set(s->flags, QUEUED)
                && !flag_is_set(s->flags, PLAYING))
            {
                continue;
            }
            // If it's about to start playing, switch it to playing.
            else if (flag_is_set(s->flags, QUEUED))
            {
                s->flags = clear_flag(s->flags, QUEUED);
                s->flags = set_flag(s->flags, PLAYING);
            }
            // If it's already playing, kill the current note and advance the
            // playhead.
            else
            {
                if (enabled && n->note_number >= 0)
                {
                    hal_send_midi(
                        USBSTANDALONE, NOTEOFF | i,
                        n->note_number, n->velocity);
                }

                s->playhead = (s->playhead + 1) % SEQUENCE_LENGTH;
            }

            // Grab the current note and play it if it's enabled.
            n = &s->notes[s->playhead];
            
            if (enabled && n->note_number >= 0)
            {
                hal_send_midi(
                    USBSTANDALONE, NOTEON | i,
                    n->note_number, n->velocity);
            }
        }
    }
}
