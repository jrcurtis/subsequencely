
#include "sequence.h"

#include "layout.h"

void layout_init(Layout* l, Scale* s, PadNotes* pn, Voices* vs)
{
    l->scale = s;
    l->pad_notes = pn;
    l->voices = vs;
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
        l->lit_pads[i] = 0x00;
    }
}

u8 layout_is_root_note(Layout* l, u8 note_number)
{
    return ((s8)note_number - l->root_note) % NUM_NOTES == 0;
}

void layout_assign_pads(Layout* l)
{
    u8 start_scale_deg = 0;
    u8 octave = l->octave;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        l->lit_pads[y] = 0x00;

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            u8 scale_deg = start_scale_deg + x;
            octave = l->octave + scale_deg / l->scale->num_notes;

            if (scale_deg >= l->scale->num_notes)
            {
                scale_deg = scale_deg % l->scale->num_notes;
            }

            u8 note_number = l->root_note + octave * NUM_NOTES;
            note_number += l->scale->offsets[scale_deg];

            (*l->pad_notes)[y][x] = note_number;
        }

        start_scale_deg += l->row_offset;
    }
}

void layout_toggle_note(Layout* l, u8 note)
{
    scale_toggle_note(l->scale, note);
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
    u8 index = FIRST_PAD;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        if ((*l->pad_notes)[y][GRID_SIZE - 1] < note_number)
        {
            index += GRID_SIZE + ROW_GAP;
            continue;
        }
        else if ((*l->pad_notes)[y][0] > note_number)
        {
            break;
        }

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            if ((*l->pad_notes)[y][x] != note_number)
            {
                index++;
                continue;
            }

            if (on)
            {
                l->lit_pads[y] |= 1 << x;
            }
            else
            {
                l->lit_pads[y] &= ~(1 << x);
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

u8 layout_handle_press(Layout* l, u8 index, u8 value, u8 midi_channel)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = (*l->pad_notes)[y][x];

        if (note_number <= MAX_NOTE)
        {
            u8 midi_message;

            if (value > 0)
            {
                midi_message = NOTEON;
                voices_add(l->voices, note_number, value);
            }
            else
            {
                midi_message = NOTEOFF;
                voices_remove(l->voices, note_number);
            }

            send_midi(
                midi_message | midi_channel,
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

u8 layout_handle_aftertouch(Layout* l, u8 index, u8 value, struct Sequence_* s)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = (*l->pad_notes)[y][x];

        if (note_number <= MAX_NOTE)
        {
            send_midi(
                POLYAFTERTOUCH | s->channel,
                note_number, value);

            if (voices_handle_aftertouch(l->voices, note_number, value))
            {
                send_midi(
                    CC | s->channel,
                    s->control_code,
                    value / s->control_div + s->control_offset);
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
    u8 index = FIRST_PAD;

    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            const u8* color = off_color;

            if (l->lit_pads[y] & (1 << x))
            {
                color = on_color;
            }
            else if (layout_is_root_note(l, (*l->pad_notes)[y][x]))
            {
                color = root_note_color;
            }

            plot_pad(index, color);
            index++;
        }

        index += ROW_GAP;
    }
}
