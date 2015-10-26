
#include "data.h"

#include "layout.h"

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
}

void layout_become_inactive(Layout* l)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        lp_lit_pads[i] = 0x00;
    }
}

void layout_set_drums(Layout* l)
{
    layout_assign_pads(l);
}

u8 layout_is_root_note(Layout* l, u8 note_number)
{
    return ((s8)note_number - l->root_note) % NUM_NOTES == 0;
}

u8 layout_get_note_number(Layout* l, u8 index)
{
    u8 x, y;
    if (!index_to_pad(index, &x, &y))
    {
        return 0xFF;
    }

    return lp_pad_notes[y][x];
}

void layout_assign_pads(Layout* l)
{
    if (l->row_offset < 0)
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
    u8 note_number = l->root_note + NUM_NOTES * l->octave;

    for (u8 y = 0; y < GRID_SIZE / 2; y++)
    {
        lp_lit_pads[y] = 0x00;

        for (u8 x = 0; x < GRID_SIZE / 2; x++)
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
    u8 start_scale_deg = 0;
    u8 octave = l->octave;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        lp_lit_pads[y] = 0x00;

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            u8 scale_deg = start_scale_deg + x;
            octave = l->octave + scale_deg / lp_scale.num_notes;

            if (scale_deg >= lp_scale.num_notes)
            {
                scale_deg = scale_deg % lp_scale.num_notes;
            }

            u8 note_number = l->root_note + octave * NUM_NOTES;
            note_number += lp_scale.offsets[scale_deg];

            lp_pad_notes[y][x] = note_number;
        }

        start_scale_deg += l->row_offset;
    }
}

void layout_toggle_note(Layout* l, u8 note)
{
    scale_toggle_note(&lp_scale, note);
    layout_assign_pads(l);
}

void layout_transpose(Layout* l, s8 direction)
{
    l->root_note = clamp(l->root_note + direction, 0, NUM_NOTES - 1);
    layout_assign_pads(l);
}

void layout_transpose_octave(Layout* l, s8 direction)
{
    l->octave = clamp(l->octave + direction, 0, NUM_OCTAVES - 1);
    layout_assign_pads(l);
}

void layout_set_row_offset(Layout* l, u8 o)
{
    l->row_offset = o;
    layout_assign_pads(l);
}

void layout_light_note(Layout* l, u8 note_number, u8 velocity, u8 on)
{
    if (l->row_offset < 0)
    {
        layout_light_drums(l, note_number, velocity, on);
    }
    else
    {
        layout_light_scale(l, note_number, velocity, on);
    }
}

#define DRUM_RANGE    (4 * NUM_NOTES + (16 - NUM_NOTES))
#define DRUM_SIZE     (GRID_SIZE / 2)
void layout_light_drums(Layout* l, u8 note_number, u8 velocity, u8 on)
{
    u8 start_note = l->root_note + l->octave * NUM_NOTES;

    if (note_number < start_note || note_number >= start_note + DRUM_RANGE)
    {
        return;
    }

    u8 offset = note_number - start_note;
    u8 quadrant = offset / NUM_NOTES;
    u8 quadrant_offset = offset - quadrant * NUM_NOTES;
    u8 quadrant_row = quadrant_offset / DRUM_SIZE;

    while (quadrant >= 0 && quadrant_offset < 16)
    {
        u8 y = quadrant_row + quadrant / 2 * DRUM_SIZE;
        u8 x = quadrant_offset % DRUM_SIZE + quadrant % 2 * DRUM_SIZE;

        lp_lit_pads[y] = assign_flag(lp_lit_pads[y], 1 << x, on);

        quadrant--;
        quadrant_offset += NUM_NOTES;
        quadrant_row += DRUM_SIZE - 1;
    }
}

void layout_light_scale(Layout* l, u8 note_number, u8 velocity, u8 on)
{
    u8 index = FIRST_PAD;

    for (u8 y = 0; y < GRID_SIZE; y++)
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

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            if (lp_pad_notes[y][x] != note_number)
            {
                index++;
                continue;
            }

            if (on)
            {
                lp_lit_pads[y] |= 1 << x;
            }
            else
            {
                lp_lit_pads[y] &= ~(1 << x);
            }

            index++;
        }

        index += ROW_GAP;
    }
}

/*******************************************************************************
 * Event handling functions
 ******************************************************************************/

u8 layout_handle_transpose(Layout* l, u8 index, u8 value)
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

u8 layout_handle_press(Layout* l, u8 index, u8 value, u8 channel)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = lp_pad_notes[y][x];

        if (note_number <= MAX_NOTE)
        {
            u8 midi_message;

            if (value > 0)
            {
                midi_message = NOTEON;
                voices_add(&lp_voices, note_number, value);
            }
            else
            {
                midi_message = NOTEOFF;
                voices_remove(&lp_voices, note_number);
            }

            send_midi(
                midi_message | channel,
                note_number, value);

            layout_light_note(l, note_number, value, value > 0);
        }
    }
    else
    {
        return 0;
    }


    return 1;
}

u8 layout_handle_aftertouch(Layout* l, u8 index, u8 value, u8 channel, u8 cc)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = lp_pad_notes[y][x];

        if (note_number <= MAX_NOTE)
        {
            send_midi(
                POLYAFTERTOUCH | channel,
                note_number, value);

            if (voices_handle_aftertouch(&lp_voices, note_number, value))
            {
                send_midi(CC | channel, cc, value);
            }
        }
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
    if (l->row_offset < 0)
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
    u8 index = FIRST_PAD;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            const u8* color = off_color;

            if (lp_lit_pads[y] & (1 << x))
            {
                color = on_color;
            }
            else if (layout_is_root_note(l, lp_pad_notes[y][x]))
            {
                color = root_note_color;
            }

            plot_pad(index, color);
            index++;
        }

        index += ROW_GAP;
    }
}

void layout_draw_drums(Layout* l)
{
    u8 index = FIRST_PAD;
    u8 quadrant = 0;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            const u8* color = off_color;

            if (lp_lit_pads[y] & (1 << x))
            {
                color = on_color;
            }
            else
            {
                color = drum_colors[quadrant];
            }

            if (x == DRUM_SIZE - 1)
            {
                quadrant++;
            }

            plot_pad(index, color);
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
