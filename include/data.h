
#ifndef DATA_H
#define DATA_H

#include "app.h"
#include "control_bank.h"
#include "keyboard.h"
#include "mod_wheel.h"
#include "scale.h"
#include "sequence.h"
#include "sequencer.h"
#include "slider.h"

/// Enum representing the different program modes.
typedef enum
{
    LP_SESSION_MODE,
    LP_NOTES_MODE,
    LP_SEQUENCER_MODE,
    LP_USER_MODE,
    LP_NUM_MODES
} LpState;

/// Global program flags.
typedef enum
{
    LP_IS_SETUP       = 1 << 0,
    LP_PORT_CHECKBOX  = 1 << 1,
    LP_TEMPO_BLINK    = 1 << 2,
    LP_POSITION_BLINK = 1 << 3,
    LP_SQR_DIRTY      = 1 << 4,
    LP_SEND_CLOCK     = 1 << 5,
    LP_ARMED          = 1 << 6,
    LP_RCV_CLOCK      = 1 << 7,
    LP_RCV_CLOCK_PORT = 1 << 8,
    LP_IS_ARP         = 1 << 9
} LpFlags;

/// The number of notes needed for all 8 sequences. Two of these are used: one
/// for the live sequences data, and one for extra storage.
#define NOTE_BANK_SIZE    (GRID_SIZE * SEQUENCE_LENGTH)

/// The size of the block of memory where the app header, and the two note
/// banks are stored, which is also used as a buffer for the serializer, which
/// is why it needs to be declared as one block of memroy.
#define LP_BUFFER_SIZE    (sizeof(uint32_t) + 2 * NOTE_BANK_SIZE * sizeof(Note))

#define APP_ID        (('S' << 8) | 'q')
#define DATA_VERSION  (2)
#define APP_HEADER    ((APP_ID << 16) | (DATA_VERSION))

/*******************************************************************************
 * Global data
 ******************************************************************************/

// Global settings
extern uint8_t lp_midi_port; // Which port to send on.
extern uint8_t lp_rcv_clock_port; // Which port to receive clock from.

// Program state
extern LpState lp_state; // What mode the program is in
extern uint16_t lp_flags; // Global flags

extern uint16_t lp_tap_tempo_timer; // Timer used to track tap tempo presses
extern uint16_t lp_tap_tempo_sum;
extern uint8_t lp_tap_tempo_counter;

// Data
extern uint8_t lp_buffer[LP_BUFFER_SIZE];
extern uint32_t* lp_app_header;
extern Note* lp_note_bank;
extern Note* lp_note_storage;

extern Scale lp_scale;
extern Voices lp_voices;
extern PadNotes lp_pad_notes;
extern Sequencer lp_sequencer;

// Setup UI elements
extern Slider lp_tempo_slider;
extern Slider lp_swing_slider;

extern Keyboard lp_keyboard;
extern Slider lp_row_offset_slider;

extern Slider lp_control_sens_slider;
extern Slider lp_control_offset_slider;

extern ControlBank lp_user_control_bank;

extern ModWheel lp_mod_wheel;

#endif
