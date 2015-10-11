
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
    Orientation orientation;

    /// The row (for horizontal faders) or column (for vertical ones) to draw
    /// this slider in.
    u8 position;

    /// The three byte color code to use when drawing.
    const u8* color;

    /// The currently held value.
    u8 value;
} Slider;

/// Initalizes the slider's state.
void slider_init(Slider* s, Orientation o, u8 p, const u8* c, u8 v);

/// Checks whether index is a valid pad for this slider, and updates the value
/// if so.
u8 slider_handle_press(Slider* s, u8 index);

/// Draws the slider using the position and color.
void slider_draw(Slider* s);

#endif
