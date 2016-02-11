
#ifndef GRID_H
#define GRID_H

#include "sequencer.h"

/// Draws the active sequence's notes layed out on a grid.
void grid_draw(Sequencer* sr);

/// Handles moving around using the arrow buttons in sequencer mode.
uint8_t grid_handle_translate(Sequencer* sr, uint8_t index, uint8_t value);

/// Zooms in and out.
uint8_t grid_handle_zoom(Sequencer* sr, uint8_t index, uint8_t value);

/// Handles all the presses.
uint8_t grid_handle_press(Sequencer* sr, uint8_t index, uint8_t value);

#endif
