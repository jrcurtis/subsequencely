
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
    CBK_SETUP_SHIFTED = 1 << 0
} ControlBankFlags;

typedef struct
{
    u8 flags;

    // User controls
    Slider sliders[GRID_SIZE];
    Checkbox checkboxes[GRID_SIZE * GRID_SIZE];

    // Setup controls
    Number control_numbers[GRID_SIZE];
    Number channel_numbers[GRID_SIZE];
    Checkbox checkbox_checkboxes[GRID_SIZE];
    Checkbox bipolar_checkboxes[GRID_SIZE];
} ControlBank;

void control_bank_init(ControlBank* cb);

void control_bank_draw(ControlBank* cb);

void control_bank_setup_draw(ControlBank* cb);

void control_bank_draw_slider(ControlBank* cb, u8 index);

u8 control_bank_handle_press(
    ControlBank* cb, u8 index, u8 value, u8 aftertouch);

u8 control_bank_setup_handle_press(ControlBank* cb, u8 index, u8 value);

#endif
