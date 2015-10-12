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
    SEQUENCER_MODE,
    NOTES_MODE,
    NUM_MODES
} State;

typedef enum
{
    IS_CONFIG = 0x01
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
PadNotes pad_notes;

// Notes config
Keyboard keyboard;
Slider row_offset_slider;

// Sequencer
Sequences sequences;
Sequencer sequencer;

/*******************************************************************************
 * App functionality
 ******************************************************************************/

void sequencer_mode_draw()
{
    sequencer_grid_draw(&sequencer);
    sequencer_play_draw(&sequencer);
}

void sequencer_config_draw()
{
    sequencer_play_draw(&sequencer);
}

void sequencer_mode_handle_press(u8 index, u8 value)
{
    sequencer_grid_handle_press(&sequencer, index, value);
    sequencer_mode_draw();
}

void sequencer_config_handle_press(u8 index, u8 value)
{
    sequencer_play_draw(&sequencer);
}


void handle_transpose(u8 index)
{
    if (index == TRANSPOSE_UP)
    {
        layout_transpose(&layout, 1);
        keyboard_update_indices(&keyboard);
    }
    else if (index == TRANSPOSE_DOWN)
    {
        layout_transpose(&layout, -1);
        keyboard_update_indices(&keyboard);
    }
    else if (index == OCTAVE_UP)
    {
        layout_transpose_octave(&layout, 1);
        keyboard_update_indices(&keyboard);
    }
    else if (index == OCTAVE_DOWN)
    {
        layout_transpose_octave(&layout, -1);
        keyboard_update_indices(&keyboard);
    }
}

void notes_mode_draw()
{
    layout_draw(&layout);
    sequencer_play_draw(&sequencer);
}

void notes_config_draw()
{
    keyboard_draw(&keyboard);
    slider_draw(&row_offset_slider);
    sequencer_play_draw(&sequencer);
}

void notes_mode_handle_press(u8 index, u8 value)
{
    layout_play(&layout, index, value, midi_channel);

    if (value == 0)
    {
        return;
    }

    handle_transpose(index);
}

void notes_config_handle_press(u8 index, u8 value)
{
    if (value == 0)
    {
        return;
    }

    if (slider_handle_press(&row_offset_slider, index))
    {
        layout_set_row_offset(&layout, row_offset_slider.value + 1);
    }
    keyboard_handle_press(&keyboard, index);
    handle_transpose(index);

    keyboard_draw(&keyboard);
    slider_draw(&row_offset_slider);
}


/*******************************************************************************
 * State management
 ******************************************************************************/

void set_state(State st, u8 cfg)
{
    if (state == st && flag_is_set(flags, IS_CONFIG) == cfg)
    {
        return;
    }

    clear_leds();

    if (st == SEQUENCER_MODE)
    {
        plot_pad(SESSION, number_colors[SEQUENCER_MODE]);

        if (cfg)
        {
            plot_setup(on_color);
            sequencer_config_draw();
        }
        else
        {
            plot_setup(number_colors[SEQUENCER_MODE]);
            sequencer_mode_draw();
        }
    }
    else if (st == NOTES_MODE)
    {
        plot_pad(NOTE, number_colors[NOTES_MODE]);

        if (cfg)
        {
            plot_setup(on_color);
            notes_config_draw();
        }
        else
        {
            plot_setup(number_colors[NOTES_MODE]);
            notes_mode_draw();
        }
    }

    state = st;
    flags = assign_flag(flags, IS_CONFIG, cfg);
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
        set_state(SEQUENCER_MODE, 0);
    }
    else if (index == NOTE && value > 0)
    {
        set_state(NOTES_MODE, 0);
    }
    else if (type == TYPESETUP && value > 0)
    {
        set_state(state, !flag_is_set(flags, IS_CONFIG));
    }
    else if (!flag_is_set(flags, IS_CONFIG))
    {
        if (state == SEQUENCER_MODE)
        {
            sequencer_mode_handle_press(index, value);
        }
        else if (state == NOTES_MODE)
        {
            notes_mode_handle_press(index, value);
        }
    }
    else
    {
        if (state == SEQUENCER_MODE)
        {
            sequencer_config_handle_press(index, value);
        }
        else if (state == NOTES_MODE)
        {
            notes_config_handle_press(index, value);
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
    if (state == NOTES_MODE && !flag_is_set(flags, IS_CONFIG))
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

    if (state == SEQUENCER_MODE && flag_is_set(sequencer.flags, DIRTY))
    {
        sequencer.flags = clear_flag(sequencer.flags, DIRTY);
        sequencer_mode_draw();
    }
}


void app_init()
{
    layout_init(&layout, &scale, &pad_notes);

    keyboard_init(&keyboard, &layout);
    slider_init(
        &row_offset_slider,
        HORIZONTAL, 2, slider_color,
        layout.row_offset - 1);

    sequencer_init(&sequencer, &sequences, &layout);

    set_state(NOTES_MODE, 0);

    layout_draw(&layout);
}
