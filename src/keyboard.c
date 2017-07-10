
#include "data.h"

#include "keyboard.h"

const uint8_t diatonic_notes[NUM_NOTES] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1};

void keyboard_update_indices(Keyboard* k)
{
    for (uint8_t i = 0; i < NUM_KEYBOARD_PADS; i++)
    {
        k->index_to_note[i] = -1;
    }

    uint8_t x = 0;
    uint8_t y = 0;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        uint8_t note = (i + k->layout->root_note) % NUM_NOTES;

        if (!diatonic_notes[note])
        {
            y++;
        }

        uint8_t index = coord_to_index(x, y);
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

uint8_t keyboard_index_is_key(Keyboard* k, uint8_t index)
{
    if (index < FIRST_KEYBOARD_PAD || index > LAST_KEYBOARD_PAD)
    {
        return 0;
    }

    return k->index_to_note[index - FIRST_KEYBOARD_PAD] != -1;
}

uint8_t keyboard_handle_press(Keyboard* k, uint8_t index, uint8_t value, uint8_t y, uint8_t is_highlight_press)
{
    index -= y * 10;
    if (value == 0 || !keyboard_index_is_key(k, index))
    {
        return 0;
    }

    uint8_t deg = (k->index_to_note[index - FIRST_KEYBOARD_PAD]
              - k->layout->root_note
              + NUM_NOTES) % NUM_NOTES;
    if (is_highlight_press) 
    {
        layout_toggle_highlight(k->layout, deg);
    } 
    else
    {
        layout_toggle_note(k->layout, deg);
    }
    return 1;
}

void keyboard_draw(Keyboard* k, uint8_t draw_highlighted)
{
    keyboard_draw_y(k, draw_highlighted, 0);
}

void keyboard_draw_y(Keyboard* k, uint8_t draw_highlighted, uint8_t y)
{
    const uint8_t* color = white_key_color;

    for (uint8_t i = 0; i < NUM_KEYBOARD_PADS; i++)
    {
        int8_t note = k->index_to_note[i];

        if (i >= GRID_SIZE && i < ROW_SIZE)
        {
            continue;
        }
        else if (note == -1)
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

        int8_t deg = (note + NUM_NOTES - k->layout->root_note) % NUM_NOTES;
        if (color != off_color && draw_highlighted && !scale_contains_note(&lp_scale, deg)) {
            color = invalid_note_color;
        }
        uint8_t is_in_scale = draw_highlighted ? scale_contains_highlight(&lp_scale, deg) : scale_contains_note(&lp_scale, deg);
        uint8_t dimness = !is_in_scale * 4;
        plot_pad_dim(i + FIRST_KEYBOARD_PAD + (10*y), color, dimness);
    }
}
