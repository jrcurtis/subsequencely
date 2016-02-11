
#ifndef SLIDER_H
#define SLIDER_H

#include "app.h"
#include "util.h"
#include "colors.h"

/// Represents an onscreen fader that increases horizontally left to right.
/// Currently spans the entire length of the grid and can be set to
/// higher resolution to represent more than 8 values.
typedef struct
{
    /// The number of divisions per pad. If this is 1, the slider ranges from
    /// 0 to 7. If it's 3, the range is 0 to 23. The different subdivisions
    /// are set by the velocity of the slider press.
    uint8_t resolution;

    /// An offset that is added to the slider value when accessed through the
    /// accessors.
    int8_t offset;

    /// The currently held value.
    int8_t value;
} Slider;

/// Initalizes the slider's state.
void slider_init(Slider* s, uint8_t resolution, int8_t offset, int8_t value);

/// Sets the value, subtracting the slider offset to keep the value normalized.
void slider_set_value(Slider* s, int8_t value);

/// Gets the value of the slider + offset.
int8_t slider_get_value(Slider* s);

/// Checks whether index is a valid pad for this slider, and updates the value
/// if so.
uint8_t slider_handle_press(Slider* s, uint8_t index, uint8_t value, uint8_t pos);

/// Draws the slider using the position and color.
void slider_draw(Slider* s, uint8_t pos, const uint8_t* color);

#endif
