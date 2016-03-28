
#include "data.h"
#include "sequence.h"

#include "grid.h"

uint8_t note_numbers[GRID_SIZE + 1];

void grid_update_cache(Sequencer* sr, int8_t translation)
{
    Sequence* s = sequencer_get_active(sr);
    uint8_t min_y = 0;
    uint8_t max_y = GRID_SIZE + 1;

    if (translation == 1)
    {
        for (uint8_t i = 0; i < GRID_SIZE; i++)
        {
            note_numbers[i] = note_numbers[i + 1];
        }

        min_y = GRID_SIZE;
    }
    else if (translation == -1)
    {
        uint8_t scale_deg = (s->y + GRID_SIZE) % lp_scale.num_notes;
        for (uint8_t i = GRID_SIZE; i > 0; i--)
        {
            note_numbers[i] = note_numbers[i - 1];
        }

        max_y = 1;
    }

    uint8_t drums = flag_is_set(s->layout.row_offset, LYT_DRUMS);
    uint8_t num_notes = drums ? NUM_NOTES : lp_scale.num_notes;
    uint8_t scale_deg;
    uint8_t octave;

    scale_deg = (s->y + min_y) % num_notes;
    octave = (s->y + min_y) / num_notes;

    for (uint8_t i = min_y; i < max_y; i++)
    {
        note_numbers[i] = (drums ? scale_deg : lp_scale.offsets[scale_deg])
            + s->layout.root_note
            + NUM_NOTES * octave;

        scale_deg++;
        if (scale_deg >= num_notes)
        {
            scale_deg = 0;
            octave++;
        }
    }
}

void grid_draw(Sequencer* sr)
{
    Sequence* s = sequencer_get_active(sr);
    uint8_t scale_deg = s->y % lp_scale.num_notes;
    uint8_t octave = s->y / lp_scale.num_notes;
    uint8_t zoom = zoom_to_sequence_x(s);

    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
        uint8_t seq_x = grid_to_sequence_x(s, x);
        Note* n;
        uint8_t y;

        for (uint8_t n_i = 0; n_i < zoom; n_i++)
        {
            n = sequence_get_note(s, seq_x + n_i);

            if (n->note_number >= note_numbers[0]
                && n->note_number < note_numbers[GRID_SIZE])
            {
                break;
            }
        }

        for (y = 0; y < GRID_SIZE; y++)
        {
            uint8_t index = FIRST_PAD + x + y * ROW_SIZE;
            if (n->note_number >= note_numbers[y]
                && n->note_number < note_numbers[y + 1])
            {
                const uint8_t* color = number_colors[seq_x & 3];
                uint8_t dimness = min(100, 127 - n->velocity) / 25;
                plot_pad_dim(index, color, dimness);
            }
            else if (s->playhead / zoom == x + s->x)
            {
                plot_pad(index, on_color);
            }
            else if (layout_is_root_note(&s->layout, note_numbers[y]))
            {
                plot_pad(index, root_note_color);
            }
            else
            {
                plot_pad(index, off_color);
            }
        }
    }
}

uint8_t grid_handle_translate(Sequencer* sr, uint8_t index, uint8_t value)
{
    if (modifier_held(LP_SHIFT) || value == 0)
    {
        return 0;
    }

    Sequence* s = sequencer_get_active(sr);

    if (index == LP_OCTAVE_UP)
    {
        if (grid_to_sequence_y(s, GRID_SIZE - 1) < MAX_NOTE)
        {
            s->y++;
            grid_update_cache(sr, 1);
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (s->y > 0)
        {
            s->y--;
            grid_update_cache(sr, -1);
        }
    }
    else if (index == LP_TRANSPOSE_UP)
    {
        if ((s->x + GRID_SIZE) * zoom_to_sequence_x(s) < SEQUENCE_LENGTH)
        {
            s->x++;
        }
    }
    else if (index == LP_TRANSPOSE_DOWN)
    {
        if (s->x > 0)
        {
            s->x--;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t grid_handle_zoom(Sequencer* sr, uint8_t index, uint8_t value)
{
    if (!modifier_held(LP_SHIFT) || value == 0)
    {
        return 0;
    }

    Sequence* s = sequencer_get_active(sr);

    if (index == LP_OCTAVE_UP)
    {
        if (s->zoom < MAX_ZOOM)
        {
            s->zoom++;
            s->x *= 2;
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (s->zoom > 0)
        {
            s->zoom--;
            s->x /= 2;
            uint8_t max_x = (GRID_SIZE << s->zoom) - GRID_SIZE;
            if (s->x > max_x)
            {
                s->x = max_x;
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t grid_handle_press(Sequencer* sr, uint8_t index, uint8_t value)
{
    uint8_t x = 0;
    uint8_t y = 0;

    if (grid_handle_zoom(sr, index, value)) { }
    else if (grid_handle_translate(sr, index, value)) { }
    else if (index == LP_DELETE && value > 0 && modifier_held(LP_SHIFT))
    {
        sequence_clear_notes(sequencer_get_active(sr));
    }
    else if (index == LP_UNDO && value > 0)
    {
        sequence_reverse(sequencer_get_active(sr));
    }
    else if (index_to_pad(index, &x, &y) && value > 0)
    {
        Sequence* s = sequencer_get_active(sr);
        uint8_t min_x = grid_to_sequence_x(s, x);
        uint8_t max_x = min_x + zoom_to_sequence_x(s);

        uint8_t min_note_number = note_numbers[y];
        uint8_t max_note_number = note_numbers[y + 1];
        
        // Holding shift creates a slide note, or a slide delete. A slide
        // note fills up all the empty steps from min_x to max_x with the
        // same note tied together. A slide delete deletes all the consecutive
        // notes with identical note number from min_x to max_x.
        uint8_t slide = modifier_held(LP_SHIFT);
        uint8_t note_deleted = 0;

        for (uint8_t i = min_x; i < max_x; i++)
        {
            Note* n = sequence_get_note(s, i);

            if (s->playhead == i)
            {
                sequence_kill_note(s, n);
            }

            // If a slide delete has already deleted one note and the next note
            // is not a tied version of the same note, then stop deleting.
            if (slide && note_deleted)
            {
                if (n->note_number == min_note_number
                    && flag_is_set(n->flags, NTE_SLIDE))
                {
                    n->note_number = -1;
                    n->velocity = 0;
                    n->flags = 0x00;
                }
                else
                {
                    break;
                }
            }
            else if (n->note_number >= min_note_number
                     && n->note_number < max_note_number)
            {
                if (slide)
                {
                    min_note_number = max_note_number = n->note_number;
                }

                n->note_number = -1;
                n->velocity = 0;
                n->flags = 0x00;
                note_deleted = 1;

                if (!slide)
                {
                    break;
                }
            }
        }

        if (note_deleted)
        {
            return 1;
        }

        uint8_t note_created = 0;

        for (uint8_t i = min_x; i < max_x; i++)
        {
            Note* n = sequence_get_note(s, i);

            // In slide mode, if a note has already been created, then keep
            // writing into subsequent steps as long as they are either empty,
            // or are the same held note as the previously overwritten value.
            if (note_created)
            {
                if (n->note_number != -1
                    && (n->note_number != max_note_number
                        || !flag_is_set(n->flags, NTE_SLIDE)))
                {
                    break;
                }
            }
            // If a note has not already been created, mark the value of the
            // first note being overwritten, so that we can tell is subsequent
            // notes are held over from this one, or are different.
            else
            {
                max_note_number = n->note_number;
            }

            n->note_number = min_note_number;
            n->velocity = value;
            n->flags = slide ? NTE_SLIDE : 0x00;
            note_created = 1;

            // A non-slide write only overwrites one step.
            if (!slide)
            {
                break;
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
