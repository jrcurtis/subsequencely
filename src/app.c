/******************************************************************************
 
 Copyright (c) 2015, Focusrite Audio Engineering Ltd.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of Focusrite Audio Engineering Ltd., nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*****************************************************************************/

/*******************************************************************************
 * Headers
 ******************************************************************************/

#include "app.h"
#include "seq.h"
#include "colors.h"

/*******************************************************************************
 * Event handlers
 ******************************************************************************/
int8_t shift_held = 0;
int8_t note_held = 0;
void app_surface_event(uint8_t type, uint8_t index, uint8_t value)
{
#ifndef SEQ_DEBUG
    modifier_index_assign(index, value > 0);

    // The play buttons are active all the time, but they are drawn first
    // in case any mode needs to overwrite them with specific info.
    sequencer_play_draw(&lp_sequencer);
    arp_draw();

    if (index == LP_SHIFT) {
        if (value > 0) {
            shift_held = 1;
        } else { 
            shift_held = 0;
        }
    }

    if (index == LP_NOTE) {
        if (value > 0) {
            note_held = 1;
        } else { 
            note_held = 0;
        }        
    }

    if (index == LP_SESSION)
    {
        if (value > 0)
        {
            set_state(LP_SESSION_MODE, 0, 0);
        }
    }
    else if (index == LP_NOTE)
    {
        if (value > 0)
        {
            set_state(LP_NOTES_MODE, 0, 0);
            plot_pad(LP_TRANSPOSE_UP, scale_button_color);
            plot_pad(LP_TRANSPOSE_DOWN, scale_button_color);
        }
        else
        {
            layout_draw_transpose_octave_buttons(sequencer_get_layout(&lp_sequencer));
        }
    }
    else if (index == LP_DEVICE)
    {
        if (value > 0)
        {
            set_state(LP_SEQUENCER_MODE, 0, 0);
        }
    }
    else if (index == LP_USER)
    {
        if (value > 0)
        {
            set_state(LP_USER_MODE, 0, 0);
        }
    }
    else if (type == TYPESETUP && value > 0)
    {
        set_state(lp_state, (shift_held && flag_is_set(lp_flags, LP_IS_SETUP)) || !flag_is_set(lp_flags, LP_IS_SETUP), shift_held);
    }
    else if (sequencer_handle_play(&lp_sequencer, index, value)) { }
    // Don't let tap tempo be used when tempo is controlled by clock.
    else if (!flag_is_set(lp_flags, LP_RCV_CLOCK)
             && tap_tempo_handle_press(index, value)) { }
    else if (arp_handle_press(index, value)) { }
    else if (!flag_is_set(lp_flags, LP_IS_SETUP))
    {
        if (lp_state == LP_SESSION_MODE)
        {
            session_mode_handle_press(index, value);
            session_mode_draw();
        }
        else if (lp_state == LP_NOTES_MODE)
        {
            notes_mode_handle_press(index, value, shift_held, note_held);
            notes_mode_draw();
        }
        else if (lp_state == LP_SEQUENCER_MODE)
        {
            sequencer_mode_handle_press(index, value);
            sequencer_mode_draw();
        }
        else if (lp_state == LP_USER_MODE)
        {
            user_mode_handle_press(index, value);
            user_mode_draw();
        }
    }
    else
    {
        if (lp_state == LP_SESSION_MODE)
        {
            session_setup_handle_press(index, value);
            session_setup_draw();
        }
        else if (lp_state == LP_NOTES_MODE)
        {
            if (flag_is_set(lp_flags, LP_IS_SETUP2)) {
                if (scales_setup_handle_press(index, value)) 
                {
                    scales_setup_draw();
                }
            } else {
                notes_setup_handle_press(index, value);
                notes_setup_draw();
            }
        }
        else if (lp_state == LP_SEQUENCER_MODE)
        {
            sequencer_setup_handle_press(index, value);
            sequencer_setup_draw();
        }
        else if (lp_state == LP_USER_MODE)
        {
            user_setup_handle_press(index, value);
            user_setup_draw();
        }
    }
#else
    send_midi(
        value > 0 ? NOTEON : NOTEOFF,
        type == TYPEPAD ? index : LP_SETUP,
        value);
#endif
}

void app_midi_event(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2)
{
#ifndef SEQ_DEBUG
    if (status == MIDITIMINGCLOCK
        && flag_is_set(lp_flags, LP_RCV_CLOCK)
        && port == lp_rcv_clock_port)
    {
        sequencer_tick(&lp_sequencer, 1);

        lp_tap_tempo_counter++;
        if (lp_tap_tempo_counter == TICKS_PER_BEAT)
        {
            // Update the tempo if it has changed enough to be significant.
            // The sequencer ticks immediately upon receiving the midi message
            // but internal timer still used for swing. step_millis must be
            // exact multiple of clock_millis so that sequence is stepped on
            // the exact step that clock is received when swing is not on.
            // With swing, some steps will fall between midi clock messages.
            int16_t tempo_diff = (int16_t)lp_tap_tempo_timer - lp_tap_tempo_sum;
            if (abs(tempo_diff) >= TICKS_PER_BEAT)
            {
                sequencer_set_tempo_millis(
                    &lp_sequencer,
                    (lp_tap_tempo_timer / TICKS_PER_BEAT) * TICKS_PER_STEP);
                lp_tap_tempo_sum = lp_tap_tempo_timer;
            }

            lp_tap_tempo_counter = 0;
            lp_tap_tempo_timer = 0;
        }
    }
#else

#endif
}


void app_sysex_event(uint8_t port, uint8_t * data, uint16_t count)
{
#ifndef SEQ_DEBUG

#else
    if (port == USBSTANDALONE && count == 9)
    {
        plot_pad(data[4], data + 5);
    }
#endif
}


void app_aftertouch_event(uint8_t index, uint8_t value)
{
#ifndef SEQ_DEBUG
    uint8_t setup = flag_is_set(lp_flags, LP_IS_SETUP);

    if (lp_state == LP_NOTES_MODE && !setup)
    {
        Sequence* s = sequencer_get_active(&lp_sequencer);
        sequence_handle_aftertouch(s, index, value);
        sequence_draw(s);
    }
    else if (lp_state == LP_SEQUENCER_MODE && setup)
    {
        if (slider_handle_press(&lp_tempo_slider, index, value, TEMPO_POS))
        {
            sequencer_set_tempo(
                &lp_sequencer,
                TEMPO_MUL * slider_get_value(&lp_tempo_slider));
            
            slider_draw(&lp_tempo_slider, TEMPO_POS, TEMPO_COLOR);
        }
    }
    else if (lp_state == LP_USER_MODE && !setup)
    {
        control_bank_handle_press(&lp_user_control_bank, index, value, 1);
        control_bank_draw_slider(&lp_user_control_bank, index);
    }
#else
    send_midi(
        POLYAFTERTOUCH,
        index, value);
#endif
}
	

void app_cable_event(uint8_t type, uint8_t value)
{
    if (type == MIDI_IN_CABLE)
    {
		
    }
    else if (type == MIDI_OUT_CABLE)
    {
		
    }
}


void app_timer_event()
{
#ifndef SEQ_DEBUG
    sequencer_tick(&lp_sequencer, 0);
    if (lp_text_displayer.active) {
        text_display_tick(&lp_text_displayer);
    }
    lp_tap_tempo_timer++;

    if (flag_is_set(lp_flags, LP_SQR_DIRTY))
    {
        lp_flags = clear_flag(lp_flags, LP_SQR_DIRTY);

        sequencer_blink_draw(
            &lp_sequencer,
            flag_is_set(lp_flags, LP_TEMPO_BLINK),
            flag_is_set(lp_flags, LP_POSITION_BLINK));

        sequencer_play_draw(&lp_sequencer);

        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {
            if (lp_state == LP_SEQUENCER_MODE)
            {
                sequencer_setup_draw();
            }
        }
        else if (lp_state == LP_SEQUENCER_MODE)
        {
            sequencer_mode_draw();
        }
        else if (lp_state == LP_SESSION_MODE)
        {
            session_mode_draw();
        }
    }
#else

#endif
}


void app_init()
{
#ifndef SEQ_DEBUG
    text_display_init(&lp_text_displayer);

    slider_init(
        &lp_tempo_slider,
        TEMPO_RESOLUTION, 60 / TEMPO_MUL + 1,
        DEFAULT_TEMPO / TEMPO_MUL);

    slider_init(
        &lp_swing_slider,
        1, -3,
        0);

    slider_init(
        &lp_row_offset_slider,
        1, 1,
        0);

    slider_init(
        &lp_control_sens_slider,
        CC_SENS_RESOLUTION, 0,
        CC_SENS_RESOLUTION * GRID_SIZE - 1);

    slider_init(
        &lp_control_offset_slider,
        16, 0,
        0);

    sequencer_init(&lp_sequencer);

    control_bank_init(&lp_user_control_bank);

    lp_mod_wheel = 0;
    lp_flags = 0;
    lp_quick_scale_current = 1;

    deserialize_app();

    set_state(LP_NOTES_MODE, 0, 0);
#else
    for (uint8_t i = 0; i < 100; i++)
    {
        uint8_t m = i % 10;
        if (i < 10 || i > 89 || m == 0 || m == 9)
        {
            hal_plot_led(TYPEPAD, i, 0x0F, 0x00, 0x00);
        }
    }
#endif
}
