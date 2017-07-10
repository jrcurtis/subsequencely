
#include "app.h"
#include "colors.h"

const uint8_t root_note_color[3] = {0x78, 0x20, 0x20};
const uint8_t white_note_color[3] = {0x10, 0x10, 0x30};
const uint8_t black_note_color[3] = {0x04, 0x04, 0x04};
const uint8_t invalid_note_color[3] = {0x3F, 0x00, 0x00};
const uint8_t c_note_color[3] = {0x02, 0x06, 0x02};
const uint8_t no_note_color[3] = {0x0F, 0x00, 0x00};

const uint8_t layout_octave_color[3] = {0x00, 0x00, 0xFF};
const uint8_t layout_transpose_color[3] = {0x00, 0xFF, 0x00};
const uint8_t scale_button_color[3] = {0x3F, 0x1F, 0x00};

const uint8_t white_key_color[3] = {0x0F, 0x2F, 0x7F};
const uint8_t black_key_color[3] = {0x3F, 0x00, 0x17};
const uint8_t slider_color[3] = {0x07, 0x7F, 0x0F};
const uint8_t off_color[3] = {0x00, 0x00, 0x00};
const uint8_t on_color[3] = {0xFF, 0xFF, 0xFF};

const uint8_t note_octave_up_colors[10][3] = {
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x02},
    {0x00, 0x00, 0x08},
    {0x00, 0x00, 0x20}, // Default Octave
    {0x00, 0x00, 0x28},
    {0x00, 0x00, 0x30},
    {0x00, 0x00, 0x3F},
    {0x10, 0x00, 0x3F},
    {0x3F, 0x00, 0x20},
    {0x3F, 0x00, 0x00}
};

const uint8_t note_octave_down_colors[10][3] = {
    {0x00, 0x00, 0x3F},
    {0x00, 0x00, 0x30},
    {0x00, 0x00, 0x28},
    {0x00, 0x00, 0x20}, // Default Octave
    {0x00, 0x00, 0x08},
    {0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00}
};

const uint8_t note_transpose_up_colors[12][3] = {
    {0x00, 0x00, 0x00}, //  0 (default / no transposition)
    {0x00, 0x04, 0x00}, // +1
    {0x00, 0x20, 0x00}, // +2
    {0x00, 0x3F, 0x00}, // +3
    {0x0F, 0x37, 0x00}, // +4
    {0x1F, 0x2F, 0x00}, // +5
    {0x2F, 0x27, 0x00}, //+-6
    {0x00, 0x00, 0x00}, // -5
    {0x00, 0x00, 0x00}, // -4
    {0x00, 0x00, 0x00}, // -3
    {0x00, 0x00, 0x00}, // -2
    {0x00, 0x00, 0x00}  // -1
};

const uint8_t note_transpose_down_colors[12][3] = {
    {0x00, 0x00, 0x00}, //  0 (default / no transposition)
    {0x00, 0x00, 0x00}, // +1
    {0x00, 0x00, 0x00}, // +2
    {0x00, 0x00, 0x00}, // +3
    {0x00, 0x00, 0x00}, // +4
    {0x00, 0x00, 0x00}, // +5
    {0x2F, 0x27, 0x00}, //+-6
    {0x1F, 0x2F, 0x00}, // -5
    {0x0F, 0x37, 0x00}, // -4
    {0x00, 0x3F, 0x00}, // -3
    {0x00, 0x20, 0x00}, // -2
    {0x00, 0x04, 0x00}  // -1
};

const uint8_t sequence_colors[8][3] = {
    {0x7F, 0x00, 0x00},
    {0x3F, 0x0F, 0x00},
    {0x3F, 0x3F, 0x00},
    {0x00, 0x7F, 0x00},
    {0x00, 0x3F, 0x3F},
    {0x00, 0x00, 0x7F},
    {0x0F, 0x00, 0x3F},
    {0x3F, 0x00, 0x3F}
};

const uint8_t number_colors[4][3] = {
    {0x10, 0x00, 0x00},
    {0x08, 0x08, 0x00},
    {0x00, 0x10, 0x00},
    {0x00, 0x00, 0x10}
};

const uint8_t drum_colors[4][3] = {
    {0x0F, 0x04, 0x04},
    {0x04, 0x0F, 0x04},
    {0x04, 0x04, 0x0F},
    {0x0F, 0x04, 0x0F}
};
