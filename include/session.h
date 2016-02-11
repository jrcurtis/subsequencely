
#ifndef SESSION_H
#define SESSION_H

#include "sequencer.h"

/// Draws the session mode view of the sequences.
void session_draw(Sequencer* sr);

/// Handles presses on the pads and the modifiers on the left side of the pads
/// in session mode.
uint8_t session_handle_press(Sequencer* sr, uint8_t index, uint8_t value);

#endif
