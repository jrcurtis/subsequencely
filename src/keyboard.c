
#include "keyboard.h"

const u8 diatonic_notes[NUM_NOTES] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1};

void keyboard_update_indices(Keyboard* k)
{
    for (u8 i = 0; i < NUM_KEYBOARD_PADS; i++)
    {
        k->index_to_note[i] = -1;
    }

    u8 x = 0;
    u8 y = 0;

    for (u8 i = 0; i < NUM_NOTES; i++)
    {
        u8 note = (i + k->layout->root_note) % NUM_NOTES;

        if (!diatonic_notes[note])
        {
            y++;
        }

        u8 index = coord_to_index(x, y);
        k->index_to_note[index - FIRST_KEYBOARD_PAD] = note;

        if (diatonic_notes[note])
        {
            if (diatonic_notes[(note + 1) % NUM_NOTES])
            {
                x++;
            }
        }
        else
        {
            y--;
            x++;
        }
    }
}

void keyboard_init(Keyboard* k, Layout* l)
{
    k->layout = l;
    keyboard_update_indices(k);
}

u8 keyboard_index_is_key(Keyboard* k, u8 index)
{
    if (index < FIRST_KEYBOARD_PAD || index > LAST_KEYBOARD_PAD)
    {
        return 0;
    }

    return k->index_to_note[index - FIRST_KEYBOARD_PAD] != -1;
}

u8 keyboard_handle_press(Keyboard* k, u8 index, u8 value)
{
    if (value == 0 || !keyboard_index_is_key(k, index))
    {
        return 0;
    }

    u8 deg = (k->index_to_note[index - FIRST_KEYBOARD_PAD]
              - k->layout->root_note
              + NUM_NOTES) % NUM_NOTES;
    layout_toggle_note(k->layout, deg);

    return 1;
}

void keyboard_draw(Keyboard* k)
{
    const u8* color = white_key_color;

    for (u8 i = 0; i < NUM_KEYBOARD_PADS; i++)
    {
        s8 note = k->index_to_note[i];
        s8 deg = (note - k->layout->root_note + NUM_NOTES) % NUM_NOTES;

        if (note == -1)
        {
            color = off_color;
        }
        else if (diatonic_notes[note])
        {
            color = white_key_color;
        }
        else
        {
            color = black_key_color;
        }

        u8 is_in_scale = scale_contains_note(k->layout->scale, deg);
        u8 dimness = !is_in_scale * 4;
        plot_pad_dim(i + FIRST_KEYBOARD_PAD, color, dimness);
    }
}
