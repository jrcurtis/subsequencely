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


/*******************************************************************************
 * Event handlers
 ******************************************************************************/

void app_surface_event(u8 type, u8 index, u8 value)
{
#ifndef SEQ_DEBUG
    modifier_index_assign(index, value > 0);

    // The play buttons are active all the time, but they are drawn first
    // in case any mode needs to overwrite them with specific info.
    sequencer_play_draw(&lp_sequencer);

    if (index == LP_SESSION)
    {
        if (value > 0)
        {
            set_state(LP_SESSION_MODE, 0);
        }
    }
    else if (index == LP_NOTE)
    {
        if (value > 0)
        {
            set_state(LP_NOTES_MODE, 0);
        }
    }
    else if (index == LP_DEVICE)
    {
        if (value > 0)
        {
            set_state(LP_SEQUENCER_MODE, 0);
        }
    }
    else if (index == LP_USER)
    {
        if (value > 0)
        {
            set_state(LP_USER_MODE, 0);
        }
    }
    else if (type == TYPESETUP && value > 0)
    {
        set_state(lp_state, !flag_is_set(lp_flags, LP_IS_SETUP));
    }
    else if (sequencer_handle_play(&lp_sequencer, index, value)) { }
    else if (tap_tempo_handle_press(index, value)) { }
    else if (!flag_is_set(lp_flags, LP_IS_SETUP))
    {
        if (lp_state == LP_SESSION_MODE)
        {
            session_mode_handle_press(index, value);
            session_mode_draw();
        }
        else if (lp_state == LP_NOTES_MODE)
        {
            notes_mode_handle_press(index, value);
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
            notes_setup_handle_press(index, value);
            notes_setup_draw();
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

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
    if (port == USBMIDI)
    {
		
    }

    if (port == DINMIDI)
    {
		
    }
}


void app_sysex_event(u8 port, u8 * data, u16 count)
{
#ifndef SEQ_DEBUG

#else
    if (port == USBSTANDALONE && count == 9)
    {
        plot_pad(data[4], data + 5);
    }
#endif
}


void app_aftertouch_event(u8 index, u8 value)
{
#ifndef SEQ_DEBUG
    u8 setup = flag_is_set(lp_flags, LP_IS_SETUP);

    if (lp_state == LP_NOTES_MODE && !setup)
    {
        sequence_handle_aftertouch(
            sequencer_get_active(&lp_sequencer), index, value);
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
	

void app_cable_event(u8 type, u8 value)
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
    sequencer_tick(&lp_sequencer);
    lp_tap_tempo_timer++;

    if (flag_is_set(lp_flags, LP_SQR_DIRTY))
    {
        lp_flags = clear_flag(lp_flags, LP_SQR_DIRTY);

        sequencer_blink_draw(
            &lp_sequencer,
            flag_is_set(lp_flags, LP_TEMPO_BLINK),
            flag_is_set(lp_flags, LP_POSITION_BLINK),
            0);

        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {

        }
        else if (lp_state == LP_SEQUENCER_MODE)
        {
            sequencer_mode_draw();
        }
        else if (lp_state == LP_SESSION_MODE)
        {
            session_mode_draw();
        }
        else if (lp_state == LP_NOTES_MODE)
        {
            notes_mode_draw();
        }
    }
#else

#endif
}


void app_init()
{
#ifndef SEQ_DEBUG
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

    set_state(LP_NOTES_MODE, 0);
#else
    for (u8 i = 0; i < 100; i++)
    {
        u8 m = i % 10;
        if (i < 10 || i > 89 || m == 0 || m == 9)
        {
            hal_plot_led(TYPEPAD, i, 0x0F, 0x00, 0x00);
        }
    }
#endif
}
