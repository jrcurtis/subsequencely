
#ifndef BUTTONS_H
#define BUTTONS_H

#include "util.h"

#define LP_FIRST_BUTTON (1)
#define LP_LAST_BUTTON  (98)

#define LP_OCTAVE_DOWN    (92)
#define LP_OCTAVE_UP      (91)
#define LP_TRANSPOSE_DOWN (93)
#define LP_TRANSPOSE_UP   (94)

#define LP_SESSION        (95)
#define LP_NOTE           (96)
#define LP_DEVICE         (97)
#define LP_USER           (98)

#define LP_SETUP          (90)
#define LP_SHIFT          (80)
#define LP_CLICK          (70)
#define LP_UNDO           (60)
#define LP_DELETE         (50)
#define LP_QUANTISE       (40)
#define LP_DUPLICATE      (30)
#define LP_DOUBLE         (20)
#define LP_RECORD         (10)

#define LP_FIRST_PLAY     (19)
#define LP_LAST_PLAY      (89)
#define LP_PLAY_GAP       (ROW_SIZE)

#define LP_RECORD_ARM     (1)
#define LP_TRACK_SELECT   (2)
#define LP_MUTE           (3)
#define LP_SOLO           (4)
#define LP_VOLUME         (5)
#define LP_PAN            (6)
#define LP_SENDS          (7)
#define LP_STOP_CLIP      (8)


/*******************************************************************************
 * Modifier buttons bit flag versions
 ******************************************************************************/

#define LP_RECORD_ARM_FLAG     (1 << 0)
#define LP_TRACK_SELECT_FLAG   (1 << 1)
#define LP_MUTE_FLAG           (1 << 2)
#define LP_SOLO_FLAG           (1 << 3)
#define LP_VOLUME_FLAG         (1 << 4)
#define LP_PAN_FLAG            (1 << 5)
#define LP_SENDS_FLAG          (1 << 6)
#define LP_STOP_CLIP_FLAG      (1 << 7)

#define LP_OCTAVE_UP_FLAG      (1 << 8)
#define LP_OCTAVE_DOWN_FLAG    (1 << 9)
#define LP_TRANSPOSE_DOWN_FLAG (1 << 10)
#define LP_TRANSPOSE_UP_FLAG   (1 << 11)

#define LP_RECORD_FLAG         (1 << 12)
#define LP_DOUBLE_FLAG         (1 << 13)
#define LP_DUPLICATE_FLAG      (1 << 14)
#define LP_QUANTISE_FLAG       (1 << 15)
#define LP_DELETE_FLAG         (1 << 16)
#define LP_UNDO_FLAG           (1 << 17)
#define LP_CLICK_FLAG          (1 << 18)
#define LP_SHIFT_FLAG          (1 << 19)

#endif
