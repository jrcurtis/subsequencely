
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
    sr->x = 0;
    sr->y = 0;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        sequence_init(&(*sr->sequences)[i]);
    }
}

void sequencer_set_octave(Sequencer* sr, u8 octave)
{
    sr->y = octave * sr->layout->scale->num_notes;
}

void sequencer_grid_draw(Sequencer* sr)
{
    u8 scale_deg = sr->y % sr->layout->scale->num_notes;
    u8 octave = sr->y / sr->layout->scale->num_notes;
    Sequence* s = &(*sr->sequences)[sr->active_sequence];
    u8 index = FIRST_PAD;

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

    u8 play_index = LAST_PLAY;
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &(*sr->sequences)[i];
        if (flag_is_set(s->flags, PLAYING))
        {
            plot_pad(play_index, sequence_colors[i]);
        }
        else
        {
            plot_pad(play_index, off_color);
        }

        play_index -= PLAY_GAP;
    }
}

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

    s->flags = toggle_flag(s->flags, PLAYING);

    if (!flag_is_set(s->flags, PLAYING))
    {
        sequence_kill_note(s, si);
        s->playhead = 0;
    }
}

void sequencer_grid_handle_press(Sequencer* sr, u8 index, u8 value)
{
    u8 x = 0;
    u8 y = 0;

    if (index_to_pad(index, &x, &y))
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

void sequencer_tick(Sequencer* sr)
{
    sr->timer++;

    if (sr->timer >= sr->tempo)
    {
        sr->timer = 0;

        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            Sequence* s = &(*sr->sequences)[i];
            Note* n = &s->notes[s->playhead];

            if (!flag_is_set(s->flags, PLAYING))
            {
                continue;
            }

            if (n->note_number >= 0)
            {
                hal_send_midi(
                    USBSTANDALONE, NOTEOFF | i,
                    n->note_number, n->velocity);
            }

            s->playhead = (s->playhead + 1) % SEQUENCE_LENGTH;
            n = &s->notes[s->playhead];
            
            if (n->note_number >= 0)
            {
                hal_send_midi(
                    USBSTANDALONE, NOTEON | i,
                    n->note_number, n->velocity);
            }
        }
    }
}
