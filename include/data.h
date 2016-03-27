
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
    LP_ARP_MASK       = 0x3 << 9
} LpFlags;

#define LP_ARP_OFFSET (9)
#define lp_is_arp()   (flag_is_set(lp_flags, LP_ARP_MASK))
#define lp_get_arp()  (get_masked(lp_flags, LP_ARP_MASK, LP_ARP_OFFSET))
#define lp_set_arp(m) (set_masked(lp_flags, LP_ARP_MASK, LP_ARP_OFFSET, (m)))

typedef enum
{
    LP_ARP_OFF,
    LP_ARP_ASC,
    LP_ARP_PING,
    LP_ARP_PLAYED,
    LP_ARP_MAX
} LpArpMode;

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
extern Scale lp_scale;
extern Voices lp_voices;
extern PadNotes lp_pad_notes;
extern NoteBank lp_note_bank;
extern NoteBank lp_note_storage;
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
