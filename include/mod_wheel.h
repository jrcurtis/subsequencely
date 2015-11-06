
#ifndef MOD_WHEEL_H
#define MOD_WHEEL_H

#include "app.h"
#include "colors.h"
#include "util.h"

#define MW_SIZE        (4)
#define MW_BITS        (14)
#define MW_BYTE_BITS   (7)
#define MW_DEFAULT     (0x2000)
#define MW_MAX         (0x3FFF)
#define MW_MIN         (0x0000)
#define MW_LSB_MASK    (0x007F)
#define MW_MSB_MASK    (0x3F80)
#define MW_LSB_NEG     (0x4000)
#define MW_MSB_NEG     (0x8000)

/// Mod wheel is a UI element consisting of 4 pads. The two outer ones are a
/// coarse control that swing the value positive or negative through the whole
/// range of the control, and the two inner pads cover half the range.
/// The 14 least significant bits store the most recent aftertouch value
/// for the coarse and fine control, and the extra two bits indicate whether
/// the most recent press was on the positive or negative side.
typedef u16 ModWheel;

int mod_wheel_handle_press(ModWheel* m, u8 index, u8 value, u8 position);
void mod_wheel_draw(ModWheel m, u8 position);

/// Combines the two bytes of the mod wheel into one good value.
u16 mod_wheel_get_value(ModWheel m);

#endif
