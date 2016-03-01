
#include "data.h"

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

    uint8_t scale_deg = (s->y + min_y) % lp_scale.num_notes;
    uint8_t octave = (s->y + min_y) / lp_scale.num_notes;

    for (uint8_t i = min_y; i < max_y; i++)
    {
        note_numbers[i] = lp_scale.offsets[scale_deg]
            + s->layout.root_note
            + NUM_NOTES * octave;

        scale_deg++;
        if (scale_deg >= lp_scale.num_notes)
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
        Note* n = sequence_get_note(s, seq_x);
        uint8_t y;

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

    /* for (uint8_t y = 0; y < GRID_SIZE; y++) */
    /* { */
    /*     uint8_t note_number = lp_scale.offsets[scale_deg] */
    /*         + s->layout.root_note */
    /*         + NUM_NOTES * octave; */

    /*     for (uint8_t x = 0; x < GRID_SIZE; x++) */
    /*     { */
    /*         uint8_t seq_x = grid_to_sequence_x(s, x); */
    /*         Note* n = sequence_get_note(s, seq_x); */
            
    /*         if (n->note_number == note_number) */
    /*         { */
    /*             const uint8_t* color = number_colors[seq_x & 3]; */
    /*             uint8_t dimness = min(100, 127 - n->velocity) / 25; */
    /*             plot_pad_dim(index, color, dimness); */
    /*         } */
    /*         else if (s->playhead / zoom == x + s->x) */
    /*         { */
    /*             plot_pad(index, on_color); */
    /*         } */
    /*         else if (layout_is_root_note(&s->layout, note_number)) */
    /*         { */
    /*             plot_pad(index, root_note_color); */
    /*         } */
    /*         else */
    /*         { */
    /*             plot_pad(index, off_color); */
    /*         } */

    /*         index++; */
    /*     } */

    /*     scale_deg++; */
    /*     if (scale_deg >= lp_scale.num_notes) */
    /*     { */
    /*         scale_deg = 0; */
    /*         octave++; */
    /*     } */

    /*     index += ROW_GAP; */
    /* } */
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
        
        uint8_t note_deleted = 0;

        for (int i = min_x; i < max_x; i++)
        {
            Note* n = sequence_get_note(s, i);
            if (n->note_number >= min_note_number
                && n->note_number < max_note_number)
            {
                n->note_number = -1;
                n->velocity = 0;
                n->flags = 0x00;
                note_deleted = 1;
            }

        }

        if (!note_deleted)
        {
            Note* n = sequence_get_note(s, min_x);
            n->note_number = min_note_number;
            n->velocity = value;
            n->flags = modifier_held(LP_SHIFT) ? NTE_SLIDE : 0x00;
        }
    }
    else
    {
        return 0;
    }


    /*     Sequence* s = sequencer_get_active(sr); */
    /*     uint8_t seq_x = grid_to_sequence_x(s, x); */
    /*     Note* n = sequence_get_note(s, seq_x); */

    /*     uint8_t note_number = grid_to_sequence_y(s, y); */

    /*     if (s->playhead == seq_x) */
    /*     { */
    /*         sequence_kill_current_note(s); */
    /*     } */

    /*     if (value == 0) */
    /*     { */

    /*     } */
    /*     else if (n->note_number == note_number) */
    /*     { */
    /*         n->note_number = -1; */
    /*         n->velocity = 0; */
    /*         n->flags = 0x00; */
    /*     } */
    /*     else */
    /*     { */
    /*         n->note_number = note_number; */
    /*         n->velocity = value; */
    /*         n->flags = modifier_held(LP_SHIFT) ? NTE_SLIDE : 0x00; */
    /*     } */
    /* } */
    /* else */
    /* { */
    /*     return 0; */
    /* } */

    return 1;
}
