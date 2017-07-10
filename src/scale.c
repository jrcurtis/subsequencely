
#include "scale.h"
#include "data.h"
#include "seq.h"

void scale_init(Scale* s)
{
    s->num_notes = NUM_NOTES;
    s->notes = 0x0FFF;
    s->notes_highlighted = 0x0AB5;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        s->offsets[i] = i;
    }
}

void scale_update_offsets(Scale* s)
{
    uint8_t offset = 0;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        if (i < s->num_notes)
        {
            while (!scale_contains_note(s, offset))
            {
                offset++;
            }

            s->offsets[i] = offset;

            offset++;
        }
        else
        {
            s->offsets[i] = -1;
        }
    }
}

uint8_t scale_contains_note(Scale* s, uint8_t note)
{
    return flag_is_set(s->notes, 1 << note);
}

uint8_t scale_contains_highlight(Scale* s, uint8_t note)
{
    return flag_is_set(s->notes_highlighted, 1 << note);
}

void scale_set_notes(Scale* s, uint16_t notes)
{
    s->notes = notes;
    s->num_notes = 0;

    for (uint8_t i = 0; i < NUM_NOTES; i++)
    {
        s->num_notes += notes & 1;
        notes >>= 1;
    }

    scale_update_offsets(s);
}

void scale_set_highlights(Scale* s, uint16_t notes_highlighted)
{
    s->notes_highlighted = notes_highlighted;
}

void scale_toggle_note(Scale* s, uint8_t note)
{
    if (note < 1 || note >= NUM_NOTES)
    {
        return;
    }

    uint16_t flag = 1 << note;

    if (flag_is_set(s->notes, flag))
    {
        s->notes = clear_flag(s->notes, flag);
        s->num_notes--;
    }
    else
    {
        s->notes = set_flag(s->notes, flag);
        s->num_notes++;
    }

    scale_update_offsets(s);
}

void scale_toggle_highlight(Scale* s, uint8_t note)
{
    if (note < 1 || note >= NUM_NOTES)
    {
        return;
    }
    s->notes_highlighted = toggle_flag(s->notes_highlighted, 1 << note);
}

const uint16_t white_and_black_key_hightlight = 0b1010110101010000;
const uint16_t all_notes_available            = 0b1111111111110000;

/// Semitone bitmaps for each of the 32 scales built in
const uint16_t quick_scale_bitfields[32] = {
    0b1011010110100000, //Minor
    0b1010110101010000, //Major
    0b1011010101100000, //Dorian
    0b1101010110100000, //Phrygian
    0b1010110101100000, //Mixolydian
    0b1011010101010000, //Melodic Minor (ascending)
    0b1011010110010000, //Harmonic Minor
    0b1011110101100000, //BeBop Dorian

    0b1001011100100000, //Blues
    0b1001010100100000, //Minor Pentatonic
    0b1011001110010000, //Hungarian Minor
    0b1011001101100000, //Ukranian Dorian
    0b1100101101010000, //Marva
    0b1101011100010000, //Todi
    0b1010101010100000, //Whole Tone
    0b1111111111110000, //Chromatic

    0b1010101101010000, //Lydian
    0b1101011010100000, //Locrain
    0b1010100101000000, //Major Pentatonic
    0b1100110110100000, //Phyrigian Dominant
    0b1101101101100000, //Half-Whole Diminished
    0b1010110101110000, //Mixolydian Bebop
    0b1101101010100000, //Super Locrian
    0b1011000110000000, //Hirajoshi

    0b1100010100100000, //In Sen
    0b1010010101000000, //Yo scale
    0b1100011000100000, //Iwato
    0b1011011011010000, //Whole Half
    0b1011010110110000, //Bebop Minor
    0b1011100101000000, //Major blues
    0b1011000101000000, //Kumoi
    0b1010110111010000  //BeBop Major
};

const char* quick_scale_names[32] = {
    "MINOR",
    "MAJOR",
    "DORIAN",
    "PHRYGIAN",
    "MIXOLYDIAN",
    "MELODIC MINOR",
    "HARMONIC MINOR",
    "BEBOP DORIAN",

    "BLUES",
    "MINOR PENTATONIC",
    "HUNGARIAN MINOR",
    "UKRANIAN DORIAN",
    "MARVA",
    "TODI",
    "WHOLE TONE",
    "CHROMATIC",

    "LYDIAN",
    "LOCRAIN",
    "MAJOR PENTATONIC",
    "PHYRIGIAN DOMINANT",
    "HALF-WHOLE DIMINISHED",
    "MIXOLYDIAN BEBOP",
    "SUPER LOCRAIN",
    "HIRAJOSHI",

    "IN SEN",
    "YO SCALE",
    "IWATO",
    "WHOLE HALF",
    "BEBOP MINOR",
    "MAJOR BLUES",
    "KUMOI",
    "BEBOP MAJOR"
};

void quick_scale_draw() {
    uint8_t scale_number = 0;
    for (uint8_t y = 7; y >= 4; y--) {
        for (uint8_t x = 0; x < 8; x++) {
            plot_pad_dim(coord_to_index(x, y), scale_button_color, scale_number == lp_quick_scale_current ? 0 : 2);
            scale_number += 1;
        }
    }
}

uint16_t quick_scale_convert_notes(uint16_t in) {
    uint16_t out = in;
    int s = sizeof(in) * 8 - 1; // extra shift needed at end
    for (in >>= 1; in; in >>= 1) {   
        out <<= 1;
        out |= in & 1;
        s--;
    }
    out <<= s; // shift when v's highest bits are zero
    return out;
}

void quick_scale_apply(Layout* l,  uint8_t is_highlight_mode) {
    if (lp_quick_scale_current < 0) 
    {
        lp_quick_scale_current = 0;
    }
    if (lp_quick_scale_current > 31) 
    {
        lp_quick_scale_current = 31;
    }

    if (is_highlight_mode) 
    {
        scale_set_notes(&lp_scale, quick_scale_convert_notes(all_notes_available));
        scale_set_highlights(&lp_scale, quick_scale_convert_notes(quick_scale_bitfields[lp_quick_scale_current]));
    } 
    else 
    {
        scale_set_notes(&lp_scale, quick_scale_convert_notes(quick_scale_bitfields[lp_quick_scale_current]));
        scale_set_highlights(&lp_scale, quick_scale_convert_notes(white_and_black_key_hightlight));
    }

    layout_assign_pads(l);
}

uint8_t quick_scale_handle_press(Layout* l, uint8_t index, uint8_t value, uint8_t is_highlight_mode) 
{
    if (value == 0 || index % 10 < 1 || index % 10 > 9 || index <= 50 || index >= 90)
    {
        return 0;
    }
    lp_quick_scale_current = (8 - index/10)*8 + (index % 10 - 1); 
    quick_scale_apply(l, is_highlight_mode);
    if (value <= 60) {
        text_display(&lp_text_displayer, quick_scale_names[lp_quick_scale_current], 4, on_color, &scales_setup_draw);
    }
    return 1;
}

uint8_t quick_scale_handle_prev_next(Layout* l, uint8_t index, uint8_t value, uint8_t note_held, uint8_t is_highlight_mode) 
{
    if (value == 0 || !note_held) 
    {
        return 0;
    } 
    else if (index == LP_TRANSPOSE_DOWN)
    {
        lp_quick_scale_current -= 1;
    }
    else if (index == LP_TRANSPOSE_UP)
    {
        lp_quick_scale_current += 1;
    }
    else
    {
        return 0;
    }
    quick_scale_apply(l, is_highlight_mode);
    return 1;
}