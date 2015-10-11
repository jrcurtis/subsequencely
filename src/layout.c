
#include "layout.h"

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

void layout_init(Layout* l, Scale* s, PadNotes* pad_notes)
{
    l->scale = s;
    scale_init(l->scale);
    l->root_note = 0;
    l->octave = 2;
    l->row_offset = 5;
    l->pad_notes = pad_notes;
    layout_assign_pads(l);
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
                hal_plot_led(TYPEPAD, index, velocity, velocity, velocity);
            }
            else if (layout_is_root_note(l, note_number))
            {
                plot_pad(index, root_note_color);
            }
            else
            {
                hal_plot_led(TYPEPAD, index, 0x00, 0x00, 0x00);
            }

            index++;
        }

        index += ROW_GAP;
    }
}

void layout_play(Layout* l, u8 index, u8 value, u8 midi_channel)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = (*l->pad_notes)[y][x];

        if (note_number > MAX_NOTE)
        {
            return;
        }

        u8 midi_message = value > 0 ? NOTEON : NOTEOFF;
        hal_send_midi(
            USBSTANDALONE, midi_message | midi_channel,
            note_number, value);

        layout_light_note(l, note_number, value, value > 0);
    }
}

void layout_aftertouch(Layout* l, u8 index, u8 value, u8 midi_channel)
{
    u8 x, y;

    if (index_to_pad(index, &x, &y))
    {
        u8 note_number = (*l->pad_notes)[y][x];

        if (note_number > MAX_NOTE)
        {
            return;
        }

        hal_send_midi(
            USBSTANDALONE, POLYAFTERTOUCH | midi_channel,
            note_number, value);
    }
}

void layout_draw(Layout* l)
{
    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            if (layout_is_root_note(l, (*l->pad_notes)[y][x]))
            {
                plot_pad(coord_to_index(x, y), root_note_color);
            }
        }
    }
}
