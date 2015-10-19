
#ifndef GRID_H
#define GRID_H

#include "sequencer.h"

u8 grid_handle_translate(Sequencer* sr, u8 index, u8 value);

u8 grid_handle_zoom(Sequencer* sr, u8 index, u8 value);

u8 grid_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
