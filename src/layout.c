
#include "data.h"

#include "layout.h"

#define DRUM_RANGE    (4 * NUM_NOTES + (16 - NUM_NOTES))
#define DRUM_SIZE     (GRID_SIZE / 2)

void layout_init(Layout* l)
{
    l->root_note = 0;
    l->octave = 2;
    l->row_offset = 5;
}

/*******************************************************************************
 * Accessor functions
 ******************************************************************************/

void layout_become_active(Layout* l)
{
    layout_assign_pads(l);

    if (lp_state == LP_NOTES_MODE && !flag_is_set(lp_flags, LP_IS_SETUP))
    {
        layout_draw(l);
    }
}

void layout_become_inactive(Layout* l)
{

}

void layout_set_drums(Layout* l)
{
    layout_assign_pads(l);
}

uint8_t layout_is_root_note(Layout* l, uint8_t note_number)
{
    return ((int8_t)note_number - l->root_note) % NUM_NOTES == 0;
}

uint8_t layout_get_note_number(Layout* l, uint8_t index)
{
    uint8_t x, y;
    if (!index_to_pad(index, &x, &y))
    {
        return 0xFF;
    }

    return lp_pad_notes[y][x];
}

void layout_assign_pads(Layout* l)
{
    if (flag_is_set(l->row_offset, LYT_DRUMS))
    {
        layout_assign_drums(l);
    }
    else
    {
        layout_assign_scale(l);
    }
}

void layout_assign_drums(Layout* l)
{
    uint8_t note_number = l->root_note + NUM_NOTES * l->octave;

    for (uint8_t y = 0; y < GRID_SIZE / 2; y++)
    {
        for (uint8_t x = 0; x < GRID_SIZE / 2; x++)
        {
            lp_pad_notes[y][x] = note_number;
            lp_pad_notes[y][x + GRID_SIZE / 2] = note_number + NUM_NOTES;
            lp_pad_notes[y + GRID_SIZE / 2][x] = note_number + 2 * NUM_NOTES;
            lp_pad_notes[y + GRID_SIZE / 2][x + GRID_SIZE / 2]
                = note_number + 3 * NUM_NOTES;

            note_number++;
        }
    }
}

void layout_assign_scale(Layout* l)
{
    uint8_t start_scale_deg = 0;
    uint8_t octave = l->octave;

    for (uint8_t y = 0; y < GRID_SIZE; y++)
    {
        for (uint8_t x = 0; x < GRID_SIZE; x++)
        {
            uint8_t scale_deg = start_scale_deg + x;
            octave = l->octave + scale_deg / lp_scale.num_notes;

            if (scale_deg >= lp_scale.num_notes)
            {
                scale_deg = scale_deg % lp_scale.num_notes;
            }

            uint8_t note_number = l->root_note + octave * NUM_NOTES;
            note_number += lp_scale.offsets[scale_deg];

            lp_pad_notes[y][x] = note_number;
        }

        start_scale_deg += l->row_offset;
    }
}

void layout_toggle_note(Layout* l, uint8_t note)
{
    scale_toggle_note(&lp_scale, note);
    layout_assign_pads(l);
}

void layout_transpose(Layout* l, int8_t direction)
{
    l->root_note = clamp(l->root_note + direction, 0, NUM_NOTES - 1);
    layout_assign_pads(l);
}

void layout_transpose_octave(Layout* l, int8_t direction)
{
    l->octave = clamp(l->octave + direction, 0, NUM_OCTAVES - 1);
    layout_assign_pads(l);
}

void layout_set_row_offset(Layout* l, uint8_t o)
{
    l->row_offset = (l->row_offset & ~ROW_OFFSET_MASK) | (o & ROW_OFFSET_MASK);
    layout_assign_pads(l);
}

void layout_light_note(Layout* l, uint8_t note_number, uint8_t on)
{
    if (lp_state != LP_NOTES_MODE || flag_is_set(lp_flags, LP_IS_SETUP))
    {
        return;
    }

    if (flag_is_set(l->row_offset, LYT_DRUMS))
    {
        layout_light_drums(l, note_number, on);
    }
    else
    {
        layout_light_scale(l, note_number, on);
    }
}

void layout_light_drums(Layout* l, uint8_t note_number, uint8_t on)
{
    uint8_t start_note = l->root_note + l->octave * NUM_NOTES;

    if (note_number < start_note || note_number >= start_note + DRUM_RANGE)
    {
        return;
    }

    uint8_t offset = note_number - start_note;
    int8_t quadrant = min(3, offset / NUM_NOTES);
    uint8_t quadrant_offset = offset - quadrant * NUM_NOTES;
    uint8_t quadrant_row = quadrant_offset / DRUM_SIZE;

    while (quadrant >= 0 && quadrant_offset < 16)
    {
        uint8_t y = quadrant_row + quadrant / 2 * DRUM_SIZE;
        uint8_t x = quadrant_offset % DRUM_SIZE + quadrant % 2 * DRUM_SIZE;

        plot_pad(coord_to_index(x, y), on ? on_color : drum_colors[quadrant]);

        quadrant--;
        quadrant_offset += NUM_NOTES;
        quadrant_row += DRUM_SIZE - 1;
    }
}

void layout_light_scale(Layout* l, uint8_t note_number, uint8_t on)
{
    uint8_t index = FIRST_PAD;
    const uint8_t* color = off_color;

    if (on)
    {
        color = on_color;
    }
    else if (layout_is_root_note(l, note_number))
    {
        color = root_note_color;
    }
    else if (note_number % NUM_NOTES == 0)
    {
        color = c_note_color;
    }
    else if (diatonic_notes[note_number % NUM_NOTES])
    {
        color = white_note_color;
    }
    else
    {
        color = black_note_color;
    }

    for (uint8_t y = 0; y < GRID_SIZE; y++)
    {
        if (lp_pad_notes[y][GRID_SIZE - 1] < note_number)
        {
            index += GRID_SIZE + ROW_GAP;
            continue;
        }
        else if (lp_pad_notes[y][0] > note_number)
        {
            break;
        }

        for (uint8_t x = 0; x < GRID_SIZE; x++)
        {
            if (lp_pad_notes[y][x] != note_number)
            {
                index++;
                continue;
            }

            plot_pad(coord_to_index(x, y), color);

            index++;
        }

        index += ROW_GAP;
    }
}

/*******************************************************************************
 * Event handling functions
 ******************************************************************************/

uint8_t layout_handle_transpose(Layout* l, uint8_t index, uint8_t value)
{
    if (value == 0)
    {
        return 0;
    }

    if (index == LP_TRANSPOSE_UP)
    {
        layout_transpose(l, 1);
    }
    else if (index == LP_TRANSPOSE_DOWN)
    {
        layout_transpose(l, -1);
    }
    else if (index == LP_OCTAVE_UP)
    {
        layout_transpose_octave(l, 1);
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        layout_transpose_octave(l, -1);
    }
    else
    {
        return 0;
    }

    return 1;
}


/*******************************************************************************
 * Drawing functions
 ******************************************************************************/

void layout_draw(Layout* l)
{
    if (flag_is_set(l->row_offset, LYT_DRUMS))
    {
        layout_draw_drums(l);
    }
    else
    {
        layout_draw_scale(l);
    }
}

void layout_draw_scale(Layout* l)
{
    uint8_t index = FIRST_PAD;

    for (uint8_t y = 0; y < GRID_SIZE; y++)
    {
        for (uint8_t x = 0; x < GRID_SIZE; x++)
        {
            const uint8_t* color = off_color;
            int8_t note_number = lp_pad_notes[y][x];

            if (layout_is_root_note(l, note_number))
            {
                color = root_note_color;
            }
            else if (note_number % NUM_NOTES == 0)
            {
                color = c_note_color;
            }
            else if (diatonic_notes[note_number % NUM_NOTES])
            {
                color = white_note_color;
            }
            else
            {
                color = black_note_color;
            }

            plot_pad(index, color);
            index++;
        }

        index += ROW_GAP;
    }
}

void layout_draw_drums(Layout* l)
{
    uint8_t index = FIRST_PAD;
    uint8_t quadrant = 0;

    for (uint8_t y = 0; y < GRID_SIZE; y++)
    {
        for (uint8_t x = 0; x < GRID_SIZE; x++)
        {
            const uint8_t* color = drum_colors[quadrant];
            plot_pad(index, color);

            if (x == DRUM_SIZE - 1)
            {
                quadrant++;
            }

            index++;
        }

        if (y == DRUM_SIZE - 1)
        {
            quadrant++;
        }
        else
        {
            quadrant--;
        }

        index += ROW_GAP;
    }
}
