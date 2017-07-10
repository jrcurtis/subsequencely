
#include "data.h"

#include "layout.h"
#include "scale.h"


#define DRUM_RANGE    (4 * NUM_NOTES + (16 - NUM_NOTES))
#define DRUM_SIZE     (GRID_SIZE / 2)

void layout_init(Layout* l)
{
    l->root_note = 0;
    l->octave = LAYOUT_DEFAULT_OCTAVE;
    l->row_offset = 5;
    l->offset_horizontal = 0;
    l->offset_vertical = 0;
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
    int8_t row_start_scale_deg = l->offset_horizontal + (l->row_offset * l->offset_vertical);
    int8_t octave;

    for (int8_t y = 0; y < GRID_SIZE; y++)
    {
        for (int8_t x = 0; x < GRID_SIZE; x++)
        {
            int8_t scale_deg = row_start_scale_deg + x;
            octave = l->octave + scale_deg / lp_scale.num_notes;
            if (scale_deg < 0) {
                octave -= 1;
            }
            scale_deg %= lp_scale.num_notes;
            while (scale_deg < 0) {
                scale_deg += lp_scale.num_notes;
            }
            lp_pad_notes[y][x] = l->root_note + (octave * NUM_NOTES) + lp_scale.offsets[scale_deg];
            lp_pad_highlights[y][x] = scale_contains_highlight(&lp_scale, lp_scale.offsets[scale_deg]);
        }
        row_start_scale_deg += l->row_offset;
    }
}

void layout_toggle_note(Layout* l, uint8_t note)
{
    scale_toggle_note(&lp_scale, note);
    layout_assign_pads(l);
}

void layout_toggle_highlight(Layout* l, uint8_t note_highlight)
{
    scale_toggle_highlight(&lp_scale, note_highlight);
    layout_assign_pads(l);
}

void layout_shift_view(Layout *l, int8_t delta_x, int8_t delta_y, int8_t reset) 
{
    if (reset) {
        if (delta_x != 0) 
        {
            l->offset_horizontal = 0;
        }
        if (delta_y != 0)
        {
            l->offset_vertical = 0;
        }
    } else {
        l->offset_horizontal = clamp(l->offset_horizontal + delta_x, -4, 4);
        l->offset_vertical = clamp(l->offset_vertical + delta_y, -4, 4);
    }
    layout_assign_pads(l);
}

void layout_transpose(Layout* l, int8_t direction)
{
    if (direction == 0) {
        l->root_note = 0;
    }
    else if (direction == -1 && l->root_note == 0 && l->octave > 0) 
    {
        l->octave -= 1;
        l->root_note = NUM_NOTES -1;
    }
    else if (direction == 1 && l->root_note == NUM_NOTES -1 && l->octave < NUM_OCTAVES - 1)
    {
        l->octave += 1;
        l->root_note = 0;
    }
    else
    {
        l->root_note = clamp(l->root_note + direction, 0, NUM_NOTES - 1);
    }
    layout_assign_pads(l);
}

void layout_transpose_octave(Layout* l, int8_t direction)
{
    if (direction == 0) {
        l->octave = LAYOUT_DEFAULT_OCTAVE;
    } else {
        l->octave = clamp(l->octave + direction, 0, NUM_OCTAVES - 1);
    }
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
    uint8_t dimness = 3;
    uint8_t draw_using_highlights = flag_is_set(sequencer_get_active(&lp_sequencer)->flags, NOTE_HIGHLIGHT_ONLY);

    if (on)
    {
        color = on_color;
        dimness = 0;
    }
    else if (layout_is_root_note(l, note_number))
    {
        color = root_note_color;
    }
    else if (diatonic_notes[note_number % NUM_NOTES])
    {
        color = white_note_color;
    }
    else
    {
        color = black_note_color;
    }

    if (note_number < 0) {
        color = invalid_note_color;
    }
    else if (note_number % NUM_NOTES == 0)
    {
        dimness = 0;
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

            if (draw_using_highlights && !layout_is_root_note(l, note_number))
            {
                if (lp_pad_highlights[y][x]) 
                {
                    color = white_note_color;
                } 
                else 
                {
                    color = black_note_color;
                }
            }
            
            plot_pad_dim(coord_to_index(x, y), color, dimness);

            index++;
        }

        index += ROW_GAP;
    }
}

/*******************************************************************************
 * Event handling functions
 ******************************************************************************/
uint8_t last_pressed = 0;
uint8_t layout_handle_transpose(Layout* l, uint8_t index, uint8_t value)
{
    if (value == 0)
    {
        last_pressed = 0;
        return 0;
    }

    if (index == LP_TRANSPOSE_UP)
    {
        if (!modifier_held(LP_SHIFT)) {
            // Transposing note values in the next/previous musical key onto the same view
            if (last_pressed == LP_TRANSPOSE_DOWN) {
                layout_transpose(l, 0); // Reset to default
            } else {
                layout_transpose(l, 1);
            }
        } else {
            // Shifting the "view" horizontally
            layout_shift_view(l, 1, 0, last_pressed == LP_TRANSPOSE_DOWN);            
        }
    }
    else if (index == LP_TRANSPOSE_DOWN)
    {
        if (!modifier_held(LP_SHIFT)) {
            if (last_pressed == LP_TRANSPOSE_UP) {
                layout_transpose(l, 0); // Reset to default
            } else {
                layout_transpose(l, -1);
            }
        } else {
            layout_shift_view(l, -1, 0, last_pressed == LP_TRANSPOSE_UP);            
        }
    }
    else if (index == LP_OCTAVE_UP)
    {
        if (!modifier_held(LP_SHIFT)) {
            if (last_pressed == LP_OCTAVE_DOWN) {
                layout_transpose_octave(l, 0); // Reset to default
            } else {
                layout_transpose_octave(l, 1);
            }
        } else {
            layout_shift_view(l, 0, 1, last_pressed == LP_OCTAVE_DOWN);            
        }
    }
    else if (index == LP_OCTAVE_DOWN)
    {
        if (!modifier_held(LP_SHIFT)) {
            if (last_pressed == LP_OCTAVE_UP) {
                layout_transpose_octave(l, 0); // Reset to default
            } else {
                layout_transpose_octave(l, -1);
            }
        } else {
            layout_shift_view(l, 0, -1, last_pressed == LP_OCTAVE_UP);            
        }
    }
    else
    {
        last_pressed = 0;
        return 0;
    }
    last_pressed = index;
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
    uint8_t draw_using_highlights = flag_is_set(sequencer_get_active(&lp_sequencer)->flags, NOTE_HIGHLIGHT_ONLY);

    for (uint8_t y = 0; y < GRID_SIZE; y++)
    {
        for (uint8_t x = 0; x < GRID_SIZE; x++)
        {
            const uint8_t* color = off_color;
            uint8_t dimness = 3;
            int8_t note_number = lp_pad_notes[y][x];

            if (layout_is_root_note(l, note_number))
            {
                color = root_note_color;
            }
            else if (draw_using_highlights)
            {
                if (lp_pad_highlights[y][x]) 
                {
                    color = white_note_color;
                } 
                else 
                {
                    color = black_note_color;
                }
            }
            else if (diatonic_notes[note_number % NUM_NOTES])
            {
                color = white_note_color;
            }
            else
            {
                color = black_note_color;
            }

            if (note_number < 0) {
                color = invalid_note_color;
            } 
            else if (note_number % NUM_NOTES == 0)
            {
                dimness = 0;
            }

            plot_pad_dim(index, color, dimness);
            index++;
        }

        index += ROW_GAP;
    }

    layout_draw_transpose_octave_buttons(l);

}

void layout_draw_transpose_octave_buttons(Layout* l)
{
    plot_pad(LP_OCTAVE_UP, note_octave_up_colors[l->octave]);
    plot_pad(LP_OCTAVE_DOWN, note_octave_down_colors[l->octave]);
    plot_pad(LP_TRANSPOSE_UP, note_transpose_up_colors[l->root_note]);
    plot_pad(LP_TRANSPOSE_DOWN, note_transpose_down_colors[l->root_note]);    
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
