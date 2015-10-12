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
u8 midi_channel = 0;

// Program state
State state = NUM_MODES;
u8 flags = 0;
u8 shift_held = 0;

// Notes
Scale scale;
Layout layout;

// Notes setup
Keyboard keyboard;
Slider row_offset_slider;

// Sequencer
Sequencer sequencer;

// Sequencer setup
Slider tempo_slider;

/*******************************************************************************
 * App functionality
 ******************************************************************************/

void session_mode_draw()
{
    
}

void session_setup_draw()
{
    
}

u8 session_mode_handle_press(u8 index, u8 value)
{
    return 0;
}

u8 session_setup_handle_press(u8 index, u8 value)
{
    return 0;
}

void sequencer_mode_draw()
{
    sequencer_grid_draw(&sequencer);
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
    if (sequencer_handle_modifiers(&sequencer, index, value)) { }
    else if (sequencer_handle_play(&sequencer, index, value)) { }
    else if (sequencer_grid_handle_press(&sequencer, index, value)) { }
    else
    {
        return 0;
    }

    sequencer_mode_draw();

    return 1;
}

u8 sequencer_setup_handle_press(u8 index, u8 value)
{
    if (sequencer_handle_modifiers(&sequencer, index, value)) { }
    else if (sequencer_handle_play(&sequencer, index, value)) { }
    else if (slider_handle_press(&tempo_slider, index, value))
    {
        sequencer.tempo = bpm_to_khz(slider_get_value(&tempo_slider));
    }
    else
    {
        return 0;
    }

    sequencer_setup_draw();

    return 1;
}


void notes_mode_draw()
{
    layout_draw(&layout);
    sequencer_play_draw(&sequencer);
}

void notes_setup_draw()
{
    keyboard_draw(&keyboard);
    slider_draw(&row_offset_slider);
    sequencer_play_draw(&sequencer);
}

u8 notes_mode_handle_press(u8 index, u8 value)
{
    if (sequencer_handle_modifiers(&sequencer, index, value)) { }
    else if (sequencer_handle_play(&sequencer, index, value)) { }
    else if (layout_handle_transpose(&layout, index, value))
    {
        keyboard_update_indices(&keyboard);
    }
    else if (layout_play(&layout, index, value, midi_channel)) { }
    else
    {
        return 0;
    }

    notes_mode_draw();

    return 1;
}

u8 notes_setup_handle_press(u8 index, u8 value)
{
    if (sequencer_handle_modifiers(&sequencer, index, value)) { }
    else if (sequencer_handle_play(&sequencer, index, value)) { }
    else if (slider_handle_press(&row_offset_slider, index, value))
    {
        layout_set_row_offset(&layout, row_offset_slider.value);
    }
    else if (layout_handle_transpose(&layout, index, value))
    {
        keyboard_update_indices(&keyboard);
    }
    else if (keyboard_handle_press(&keyboard, index, value)) { }
    else
    {
        return 0;
    }

    notes_setup_draw();

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

    clear_leds();

    if (st == SESSION_MODE)
    {
        plot_pad(SESSION, number_colors[SESSION_MODE]);

        if (setup)
        {
            plot_setup(on_color);
            session_setup_draw();
        }
        else
        {
            plot_setup(number_colors[SESSION_MODE]);
            session_mode_draw();
        }
    }
    else if (st == NOTES_MODE)
    {
        plot_pad(NOTE, number_colors[NOTES_MODE]);

        if (setup)
        {
            plot_setup(on_color);
            notes_setup_draw();
        }
        else
        {
            plot_setup(number_colors[NOTES_MODE]);
            notes_mode_draw();
        }
    }
    else if (st == SEQUENCER_MODE)
    {
        plot_pad(DEVICE, number_colors[SEQUENCER_MODE]);

        if (setup)
        {
            plot_setup(on_color);
            sequencer_setup_draw();
        }
        else
        {
            plot_setup(number_colors[SEQUENCER_MODE]);
            sequencer_mode_draw();
        }
    }

    state = st;
    flags = assign_flag(flags, IS_SETUP, setup);
}

/*******************************************************************************
 * Event handlers
 ******************************************************************************/

void app_surface_event(u8 type, u8 index, u8 value)
{
    if (index == SHIFT)
    {
        shift_held = value > 0;
    }
    else if (index == SESSION && value > 0)
    {
        set_state(SESSION_MODE, 0);
    }
    else if (index == NOTE && value > 0)
    {
        set_state(NOTES_MODE, 0);
    }
    else if (index == DEVICE && value > 0)
    {
        set_state(SEQUENCER_MODE, 0);
    }
    else if (type == TYPESETUP && value > 0)
    {
        set_state(state, !flag_is_set(flags, IS_SETUP));
    }
    else if (!flag_is_set(flags, IS_SETUP))
    {
        if (state == SESSION_MODE)
        {
            session_mode_handle_press(index, value);
        }
        else if (state == NOTES_MODE)
        {
            notes_mode_handle_press(index, value);
        }
        else if (state == SEQUENCER_MODE)
        {
            sequencer_mode_handle_press(index, value);
        }
    }
    else
    {
        if (state == SESSION_MODE)
        {
            session_setup_handle_press(index, value);
        }
        else if (state == NOTES_MODE)
        {
            notes_setup_handle_press(index, value);
        }
        else if (state == SEQUENCER_MODE)
        {
            sequencer_setup_handle_press(index, value);
        }
    }
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
	
}


void app_aftertouch_event(u8 index, u8 value)
{
    if (state == NOTES_MODE && !flag_is_set(flags, IS_SETUP))
    {
        layout_aftertouch(&layout, index, value, midi_channel);
    }
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
    sequencer_tick(&sequencer);

    if (state == SEQUENCER_MODE
        && !flag_is_set(flags, IS_SETUP)
        && flag_is_set(sequencer.flags, DIRTY))
    {
        sequencer.flags = clear_flag(sequencer.flags, DIRTY);
        sequencer_mode_draw();
    }
}


void app_init()
{
    layout_init(&layout, &scale);

    keyboard_init(&keyboard, &layout);
    slider_init(
        &row_offset_slider,
        HORIZONTAL, 2, slider_color,
        1, 0,
        layout.row_offset);

    sequencer_init(&sequencer, &layout);
    slider_init(
        &tempo_slider,
        HORIZONTAL, 7, slider_color,
        15, 60,
        DEFAULT_TEMPO);

    set_state(NOTES_MODE, 0);
}
