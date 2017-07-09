
#include "util.h"

#include "buttons.h"

uint32_t lp_modifiers = 0x00000000;

uint8_t index_to_pad(uint8_t i, uint8_t* x, uint8_t* y)
{
    static uint8_t last_i = 0xFF;
    static uint8_t last_x = 0;
    static uint8_t last_y = 0;

    if (i == last_i)
    {
        *x = last_x;
        *y = last_y;
        return 1;
    }
    else if (i < FIRST_PAD || i > LAST_PAD)
    {
        return 0;
    }

    uint8_t mod = i % (ROW_SIZE);

    if (mod == 0 || mod == ROW_SIZE - 1)
    {
        return 0;
    }

    *x = mod - 1;
    *y = i / ROW_SIZE - 1;

    last_i = i;
    last_x = *x;
    last_y = *y;

    return 1;
}

void clear_leds()
{
    for (uint8_t i = LP_FIRST_BUTTON; i <= LP_LAST_BUTTON; i++)
    {
        hal_plot_led(TYPEPAD, i, 0x00, 0x00, 0x00);
    }
}

void clear_pad_leds()
{
    uint8_t index = FIRST_PAD;

    for (uint8_t i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        if (i > 0 && i % GRID_SIZE == 0)
        {
            index += ROW_GAP;
        }
        
        hal_plot_led(TYPEPAD, index, 0x00, 0x00, 0x00);

        index++;
    }
}

void modifier_index_assign(uint8_t index, uint8_t value)
{
    uint32_t flag = 1;

    if (index <= LP_STOP_CLIP)
    {
        flag <<= index - 1;
    }
    else if (index >= LP_OCTAVE_UP)
    {
        flag <<= index - LP_OCTAVE_UP + LP_STOP_CLIP;
    }
    else if (index >= LP_RECORD
             && index <= LP_SHIFT
             && (index - LP_RECORD) % ROW_SIZE == 0)
    {
        flag <<= (index - LP_RECORD) / ROW_SIZE
            + LP_STOP_CLIP
            + (LP_TRANSPOSE_UP - LP_OCTAVE_UP + 1);
    }
    else
    {
        return;
    }

    lp_modifiers = assign_flag(lp_modifiers, flag, value);
}

const uint16_t font_4x4[64] = {
    0b0000000000000000, // space
    0b0100010000000100, // !
    0b1010101000000000, // "
    0b1010000010100000, // #
    0b0100111011100100, // $ 
    0b1001001001001001, // %
    0b0010011110100110, // &
    0b0100010000000000, // '
    0b0010010001000010, // (
    0b0100001000100100, // )
    0b1010010010100000, // *
    0b0100111001000000, // +
    0b0000000000100100, // ,
    0b0000111000000000, // -
    0b0000000000000100, // .
    0b0010010001001000, // / (forward slash)
    0b0100101010100100, // 0
    0b1100010001000100, // 1
    0b0100101001001110, // 2
    0b1110011000101110, // 3
    0b1010111000100010, // 4
    0b1110100001101110, // 5
    0b1000110010101110, // 6
    0b1110001001000100, // 7
    0b1110111011101110, // 8
    0b1110101001100010, // 9
    0b0100000001000000, // :
    0b0100000001001000, // ;
    0b0010110000100000, // <
    0b1110000011100000, // =
    0b1000011010000000, // > 
    0b0100101001100100, // ?

    0b0111101110000110, // @
    0b1110101011101010, // A
    0b1000111010101110, // B
    0b0110100010000110, // C
    0b1100101010101100, // D
    0b1110100011001110, // E
    0b1110100011001000, // F
    0b0110100010100110, // G
    0b1010101011101010, // H
    0b0100010001000100, // I
    0b0010001000100110, // J
    0b1010110010101010, // K
    0b0100010001000110, // L
    0b1110111010101010, // M
    0b1010111011101010, // N
    0b1110101010101110, // O
    0b1110101011101000, // P
    0b1110101010101111, // Q
    0b1100101011001010, // R
    0b1110100000101110, // S
    0b1110010001000100, // T
    0b1010101010101110, // U
    0b1010101010100100, // V
    0b1010101011101110, // W
    0b1010010001001010, // X
    0b1010101001000100, // Y
    0b1110001010001110, // Z
    0b0110010001000110, // [
    0b1000010001000010, // \ (backslash)
    0b0110001000100110, // ]
    0b0100101000000000, // ^
    0b0000000000001110 // _
};

void text_display_init(TextDisplayer* td) {
    td->active = 0;
    td->text = 0;
    td->y = 0;
    td->color = on_color;
    td->callback_finished = 0;
    td->text_visible[0] = ' ';
    td->text_visible[1] = ' ';
    td->text_visible[2] = ' ';
}

void text_display_tick(TextDisplayer* td) {
    if (!td->active) {
        return;
    }
    if (td->frame_millis_remain > 0) {
        td->frame_millis_remain -= 1;
        return;
    }
    // Next frame 
    td->frame_millis_remain = td->millis_per_frame;
    td->frame_offset += 1;
    if (td->frame_offset == 4) {
        td->frame_offset = 0;
        td->text_visible[0] = td->text_visible[1];
        td->text_visible[1] = td->text_visible[2];
        td->text_visible[2] = td->text[td->text_index];
        if (td->text_visible[2] != 0) {
            td->text_index += 1;
        }
    }

    text_draw(td);

    if(td->text_visible[0] == 0) {
        td->active = 0;
        if(td->callback_finished != 0) {
            (*td->callback_finished)(); // Call text display finish callback, if one is set
        }
        return;
    }

}

void text_draw(TextDisplayer *td) {
    text_draw_letter(td->text_visible[0],  -(td->frame_offset), td->y, td->color);
    text_draw_letter(td->text_visible[1], 4-(td->frame_offset), td->y, td->color);
    text_draw_letter(td->text_visible[2], 8-(td->frame_offset), td->y, td->color);
}

void text_draw_letter(char c, int8_t x, int8_t y, const uint8_t *color) {
    if (x <= -4 || x >= 8 || y > 4) {
        return; // Letter not visible
    }
    if (c < 32) {
        c = 32;
    } else if (c >= 96) {
        c &= 0x5F; // Convert lower case to upper case
    }
    uint16_t letter = font_4x4[c-32];
    for (int8_t fy = 3; fy >= 0; fy--) {
       for (int8_t fx = 0; fx < 4; fx++) {
           uint16_t pixel_on = letter & 0x8000;
           letter = letter << 1;
           if (x + fx < 0 || x + fx >= 8) {
               continue;
           }
           uint8_t coord = coord_to_index(x + fx, y + fy);
           if (pixel_on) {
               plot_pad(coord, color);
           } else {
               plot_pad(coord, off_color);
           }
       }
    }
}

void text_display(TextDisplayer *td, const char *text, uint8_t y, const uint8_t *color, void (*callback_finished)(void)) {
    td->active = 0;
    
    td->text = text;
    td->y = y;
    td->color = color;
    td->text_visible[0] = ' ';
    td->text_visible[1] = ' ';
    td->text_visible[2] = ' ';

    td->text_index = 0;
    td->millis_per_frame = 90;
    td->frame_millis_remain = 0;
    td->frame_offset = 3;

    td->callback_finished = callback_finished;
    td->active = 1;
}

void text_display_stop(TextDisplayer* td) {
    td->active = 0;
}
