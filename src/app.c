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

typedef enum
{
    SESSION_MODE,
    NOTES_MODE,
    SEQUENCER_MODE,
    NUM_MODES
} State;

typedef enum
{
    IS_SETUP = 0x01
} Flags;


/*******************************************************************************
 * Global data
 ******************************************************************************/

// Global settings
u8 midi_port = USBMIDI;

// Program state
State state = NUM_MODES;
u8 flags = 0;
u16 tap_tempo_timer = 1000;
u16 tap_tempo_sum = 0;
u8 tap_tempo_counter = 0;

// Notes setup
Slider row_offset_slider;
Checkbox port_checkbox;

// Sequencer
Sequencer sequencer;

// Sequencer setup
Slider tempo_slider;

/*******************************************************************************
 * App functionality
 ******************************************************************************/

u8 tap_tempo_handle_press(u8 index, u8 value)
{
    if (index != LP_CLICK)
    {
        return 0;
    }

    if (value != 0)
    {
        if (tap_tempo_timer < 1000)
        {
            tap_tempo_sum += tap_tempo_timer;
            tap_tempo_counter++;
        }
        else
        {
            tap_tempo_sum = 0;
            tap_tempo_counter = 0;
        }

        tap_tempo_timer = 0;
    }
    else
    {
        if (tap_tempo_timer < 1000)
        {
            if (tap_tempo_counter >= 3)
            {
                sequencer.tempo = tap_tempo_sum / tap_tempo_counter
                    / STEPS_PER_PAD;
            }
        }
        else
        {
            tap_tempo_sum = 0;
            tap_tempo_counter = 0;
        }
    }

    return 0;
}

void session_mode_become_active()
{
    
}

void session_mode_become_inactive()
{
    
}

void session_setup_become_active()
{
    
} 

void session_setup_become_inactive()
{
    
}

void session_mode_draw()
{
    session_draw(&sequencer);
}

void session_setup_draw()
{
    
}

u8 session_mode_handle_press(u8 index, u8 value)
{
    if (session_handle_press(&sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 session_setup_handle_press(u8 index, u8 value)
{
    return 0;
}

void sequencer_mode_become_active()
{
    
}

void sequencer_mode_become_inactive()
{
    
}

void sequencer_setup_become_active()
{
    
} 

void sequencer_setup_become_inactive()
{
    
}

void sequencer_mode_draw()
{
    grid_draw(&sequencer);
    sequencer_play_draw(&sequencer);
}

void sequencer_setup_draw()
{
    sequencer_play_draw(&sequencer);

    slider_set_value(&tempo_slider, khz_to_bpm(sequencer.tempo));
    slider_draw(&tempo_slider);
}

u8 sequencer_mode_handle_press(u8 index, u8 value)
{
    if (grid_handle_press(&sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequencer_setup_handle_press(u8 index, u8 value)
{
    if (slider_handle_press(&tempo_slider, index, value))
    {
        sequencer.tempo = bpm_to_khz(slider_get_value(&tempo_slider));
    }
    else
    {
        return 0;
    }

    return 1;
}

void notes_mode_become_active()
{
    
}

void notes_mode_become_inactive()
{
    
}

void notes_setup_become_active()
{
    slider_set_value(
        &row_offset_slider,
        sequencer_get_layout(&sequencer)->row_offset);
} 

void notes_setup_become_inactive()
{
    
}

void notes_mode_draw()
{
    layout_draw(sequencer_get_layout(&sequencer));
}

void notes_setup_draw()
{
    keyboard_draw(&sequencer.keyboard);
    slider_draw(&row_offset_slider);
    checkbox_draw(&port_checkbox);
}

u8 notes_mode_handle_press(u8 index, u8 value)
{
    Layout* l = sequencer_get_layout(&sequencer);

    if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&sequencer.keyboard);
    }
    else if (layout_play(
                 l, index, value,
                 sequencer_get_active(&sequencer)->channel))
    {
        if (value > 0)
        {
            sequencer_handle_record(&sequencer);
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 notes_setup_handle_press(u8 index, u8 value)
{
    Layout* l = sequencer_get_layout(&sequencer);

    if (slider_handle_press(&row_offset_slider, index, value))
    {
        layout_set_row_offset(l, row_offset_slider.value);
    }
    else if (checkbox_handle_press(&port_checkbox, index, value))
    {
        midi_port = port_checkbox.value ? DINMIDI : USBMIDI;
    }
    else if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&sequencer.keyboard);
    }
    else if (keyboard_handle_press(&sequencer.keyboard, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}


/*******************************************************************************
 * State management
 ******************************************************************************/

void set_state(State st, u8 setup)
{
    if (state == st && flag_is_set(flags, IS_SETUP) == setup)
    {
        return;
    }

    if (state == SESSION_MODE)
    {
        if (setup)
        {
            session_setup_become_inactive();
        }
        else
        {
            session_mode_become_inactive();
        }
    }
    else if (state == NOTES_MODE)
    {
        if (setup)
        {
            notes_setup_become_inactive();
        }
        else
        {
            notes_mode_become_inactive();
        }
    }
    else if (state == SEQUENCER_MODE)
    {
        if (setup)
        {
            sequencer_setup_become_inactive();
        }
        else
        {
            sequencer_mode_become_inactive();
        }
    }

    clear_leds();

    if (st == SESSION_MODE)
    {
        plot_pad(LP_SESSION, number_colors[SESSION_MODE]);

        if (setup)
        {
            session_setup_become_active();
            plot_setup(on_color);
            session_setup_draw();
        }
        else
        {
            session_mode_become_active();
            plot_setup(number_colors[SESSION_MODE]);
            session_mode_draw();
        }
    }
    else if (st == NOTES_MODE)
    {
        plot_pad(LP_NOTE, number_colors[NOTES_MODE]);

        if (setup)
        {
            notes_setup_become_active();
            plot_setup(on_color);
            notes_setup_draw();
        }
        else
        {
            notes_mode_become_active();
            plot_setup(number_colors[NOTES_MODE]);
            notes_mode_draw();
        }
    }
    else if (st == SEQUENCER_MODE)
    {
        plot_pad(LP_DEVICE, number_colors[SEQUENCER_MODE]);

        if (setup)
        {
            sequencer_setup_become_active();
            plot_setup(on_color);
            sequencer_setup_draw();
        }
        else
        {
            sequencer_mode_become_active();
            plot_setup(number_colors[SEQUENCER_MODE]);
            sequencer_mode_draw();
        }
    }

    sequencer_play_draw(&sequencer);

    state = st;
    flags = assign_flag(flags, IS_SETUP, setup);
}

/*******************************************************************************
 * Event handlers
 ******************************************************************************/

void app_surface_event(u8 type, u8 index, u8 value)
{
#ifndef SEQ_DEBUG
    modifier_index_assign(index, value > 0);

    if (index == LP_SESSION && value > 0)
    {
        set_state(SESSION_MODE, 0);
    }
    else if (index == LP_NOTE && value > 0)
    {
        set_state(NOTES_MODE, 0);
    }
    else if (index == LP_DEVICE && value > 0)
    {
        set_state(SEQUENCER_MODE, 0);
    }
    else if (type == TYPESETUP && value > 0)
    {
        set_state(state, !flag_is_set(flags, IS_SETUP));
    }
    else if (sequencer_handle_play(&sequencer, index, value)) { }
    else if (tap_tempo_handle_press(index, value)) { }
    else if (!flag_is_set(flags, IS_SETUP))
    {
        if (state == SESSION_MODE)
        {
            session_mode_handle_press(index, value);
            session_mode_draw();
        }
        else if (state == NOTES_MODE)
        {
            notes_mode_handle_press(index, value);
            notes_mode_draw();
        }
        else if (state == SEQUENCER_MODE)
        {
            sequencer_mode_handle_press(index, value);
            sequencer_mode_draw();
        }
    }
    else
    {
        if (state == SESSION_MODE)
        {
            session_setup_handle_press(index, value);
            session_setup_draw();
        }
        else if (state == NOTES_MODE)
        {
            notes_setup_handle_press(index, value);
            notes_setup_draw();
        }
        else if (state == SEQUENCER_MODE)
        {
            sequencer_setup_handle_press(index, value);
            sequencer_setup_draw();
        }
    }

    sequencer_play_draw(&sequencer);
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
    if (state == NOTES_MODE && !flag_is_set(flags, IS_SETUP))
    {
        layout_aftertouch(
            sequencer_get_layout(&sequencer),
            index, value,
            sequencer_get_active(&sequencer)->channel);
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
    sequencer_tick(&sequencer);
    tap_tempo_timer++;

    if (!flag_is_set(flags, IS_SETUP)
        && flag_is_set(sequencer.flags, SQR_DIRTY))
    {
        sequencer.flags = clear_flag(sequencer.flags, SQR_DIRTY);
        if (state == SEQUENCER_MODE)
        {
            sequencer_mode_draw();
        }
        else if (state == SESSION_MODE)
        {
            session_mode_draw();
        }
        else if (state == NOTES_MODE)
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
    sequencer_init(&sequencer);
    slider_init(
        &tempo_slider,
        HORIZONTAL, 7, slider_color,
        15, 60,
        DEFAULT_TEMPO);

    slider_init(
        &row_offset_slider,
        HORIZONTAL, 2, slider_color,
        1, 0,
        0);
    checkbox_init(&port_checkbox, coord_to_index(0, 3), 0);

    set_state(NOTES_MODE, 0);
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
