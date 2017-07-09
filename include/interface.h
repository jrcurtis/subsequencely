
#ifndef INTERFACE_H
#define INTERFACE_H

// Where stuff should be positioned, and what color it should be.

#define CC_SENS_RESOLUTION         (2)
#define CC_SENS_POS                (6)
#define CC_SENS_POS_COLOR          (sequence_colors[6])
#define CC_SENS_NEG_COLOR          (sequence_colors[7])

#define CC_OFFSET_POS              (5)
#define CC_OFFSET_COLOR            (sequence_colors[2])

#define CC_POS                     (82)
#define CC_COLOR                   (number_colors[3])

#define CC_BITS                    (7)
#define CC_X                       (1)
#define CHANNEL_BITS               (4)
#define CHANNEL_X                  (4)

#define SAVE_BUTTON_INDEX          (81)
#define CLEAR_BUTTON_INDEX         (11)

#define CHECKBOX_ROW               (7)
#define CHECKBOX_ROW_INDEX         (81)
#define CHECKBOX_CHECKBOX_X        (0)
#define BIPOLAR_CHECKBOX_X         (2)

#define TEMPO_POS                  (7)
#define TEMPO_COLOR                (sequence_colors[3])
#define TEMPO_RESOLUTION           (5)
#define TEMPO_MUL                  (3)
#define BLINK_CHECKBOX_POS         (51)
#define PLAYHEAD_CHECKBOX_POS      (61)
#define PORT_CHECKBOX_POS          (11)
#define CLOCK_CHECKBOX_POS         (13)
#define RCV_CLOCK_CHECKBOX_POS     (15)
#define RCV_CLOCK_PORT_CHECKBOX_POS (25)

#define SWING_POS                  (6)
#define SWING_COLOR                (sequence_colors[5])

#define ROW_OFFSET_POS             (2)
#define ROW_OFFSET_COLOR           (number_colors[2])

#define VELOCITY_CHECKBOX_POS      (41)
#define MOD_WHEEL_CHECKBOX_POS     (43)
#define MOD_CC_CHECKBOX_POS        (53)
#define DRUM_CHECKBOX_POS          (45)
#define MULTICHANNEL_CHECKBOX_POS  (55)
#define CONTROL_CHECKBOX_POS       (81)

#define NOTE_HIGHLIGHT_ONLY_POS    (28)
#define SCALE_HIGHLIGHT_ONLY_POS     (48)
#define SCALE_HIGHLIGHT_ONLY_INV_POS (28)

#define MOD_WHEEL_POS              (51)
#define MOD_WHEEL_X                (0)
#define MOD_WHEEL_Y                (4)

#endif
