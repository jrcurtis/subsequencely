
#ifndef SLIDER_H
#define SLIDER_H

#include "app.h"
#include "util.h"
#include "colors.h"

typedef enum
{
    VERTICAL,
    HORIZONTAL
} Orientation;

/// Represents an onscreen fader that can be either horizontal or vertical.
/// Currently spans the entire length of the grid and can only be set to 8
/// discrete values, but could be updated to interpolate between them.
typedef struct
{
    /// Whether the slider is drawn horizontally or vertically.
    u8 orientation;

    /// The row (for horizontal faders) or column (for vertical ones) to draw
    /// this slider in.
    u8 position;

    /// The number of divisions per pad. If this is 1, the slider ranges from
    /// 1 to 8. If it's 3, the range is 1 to 24. The different subdivisions
    /// are set by the velocity of the slider press.
    u8 resolution;

    /// An offset that is added to the slider value when accessed through the
    /// accessors.
    s16 offset;

    /// The three byte color code to use when drawing.
    const u8* color;

    /// The currently held value.
    s16 value;
} Slider;

/// Initalizes the slider's state.
void slider_init(Slider* s, Orientation orientation, u8 position,
                 const u8* color, u8 resolution, s16 offset, s16 value);

/// Sets the value, subtracting the slider offset to keep the value normalized.
void slider_set_value(Slider* s, s16 value);

/// Gets the value of the slider + offset.
s16 slider_get_value(Slider* s);

/// Checks whether index is a valid pad for this slider, and updates the value
/// if so.
u8 slider_handle_press(Slider* s, u8 index, u8 value);

/// Draws the slider using the position and color.
void slider_draw(Slider* s);

#endif
