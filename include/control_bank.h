
#ifndef CONTROL_BANK_H
#define CONTROL_BANK_H

#include "app.h"
#include "buttons.h"
#include "checkbox.h"
#include "interface.h"
#include "number.h"
#include "slider.h"
#include "util.h"

typedef enum
{
    CBK_SETUP_SHIFTED   = 1 << 0, // Whether to show the second page of settings
    CBK_SHOW_CHECKBOXES = 1 << 1 // Whether to fill row 1 with checkboxes
} ControlBankFlags;

typedef struct
{
    uint8_t flags;

    // User controls
    Slider sliders[GRID_SIZE];
    Checkbox checkboxes;

    // Setup controls
    Number control_numbers[GRID_SIZE];
    Number channel_numbers[GRID_SIZE];
    Checkbox bipolar_checkboxes;
} ControlBank;

/// Sets up the sliders to default channels/CCs.
void control_bank_init(ControlBank* cb);

/// Draws the sliders/checkboxes.
void control_bank_draw(ControlBank* cb);

/// Draws the widgets to configure the sliders.
void control_bank_setup_draw(ControlBank* cb);

/// Draws one slider that has been changed. Used on aftertouch events to avoid
/// redrawing every slider.
void control_bank_draw_slider(ControlBank* cb, uint8_t index);

/// A press in the normal control mode.
uint8_t control_bank_handle_press(
    ControlBank* cb, uint8_t index, uint8_t value, uint8_t aftertouch);

/// A press on the configure screen.
uint8_t control_bank_setup_handle_press(ControlBank* cb, uint8_t index, uint8_t value);

#endif
