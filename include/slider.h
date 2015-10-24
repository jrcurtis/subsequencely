
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
    u8 resolution;

    /// An offset that is added to the slider value when accessed through the
    /// accessors.
    s8 offset;

    /// The currently held value.
    s8 value;
} Slider;

/// Initalizes the slider's state.
void slider_init(Slider* s, u8 resolution, s8 offset, s8 value);

/// Sets the value, subtracting the slider offset to keep the value normalized.
void slider_set_value(Slider* s, s8 value);

/// Gets the value of the slider + offset.
s8 slider_get_value(Slider* s);

/// Checks whether index is a valid pad for this slider, and updates the value
/// if so.
u8 slider_handle_press(Slider* s, u8 index, u8 value, u8 pos);

/// Draws the slider using the position and color.
void slider_draw(Slider* s, u8 pos, const u8* color);

#endif
