
#include <iostream>
#include <glm/glm.hpp>

#include <cinder/Log.h>

#include "VirtualLpp.h"

using namespace glm;
using namespace std::placeholders;

void handleMidiError(RtMidiError::Type type, const string& errorText)
{
    CI_LOG_I("midi error: " << errorText << "\n");
}

void receiveMidiCallback(double timestamp, vector<unsigned char>* message, void* userData)
{
    ((VirtualLpp*)userData)->receiveMidi(timestamp, message, nullptr);
}

void receiveMidiControlCallback(double timestamp, vector<unsigned char>* message, void* userData)
{
    ((VirtualLpp*)userData)->receiveMidiControl(timestamp, message, nullptr);
}

VirtualLpp* VirtualLpp::instance = nullptr;

const char* VirtualLpp::VIRTUAL_IN_NAME = "VirtualLpp Input";
const char* VirtualLpp::VIRTUAL_OUT_NAME = "VirtualLpp Output";


VirtualLpp::VirtualLpp(int width)
    : heldIndex(-1),
      heldVelocity(-1),
      midiOut(MidiConnection::OUTPUT, "VirtualLpp Output"),
      midiIn(MidiConnection::INPUT, "VirtualLpp Input"),
      pads{
        nullptr,   "Arm", "Select",  "Mute",  "Solo",  "Volume",   "Pan",  "Sends",  "Stop",  "",
            "O", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
       "Double", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
    "Duplicate", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
     "Quantise", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
       "Delete", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
         "Undo", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Click", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Shift", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Setup",     "^",      "v",     "<",     ">", "Session",  "Note", "Device",  "User"
      }
{
    VirtualLpp::instance = this;
    setWidth(width);

    midiOut.getOutPort()->setErrorCallback(handleMidiError);

    shared_ptr<RtMidiIn> inPort = midiIn.getInPort();
    inPort->setErrorCallback(handleMidiError);
    inPort->setCallback(receiveMidiCallback, this);
    inPort->ignoreTypes(true, false, true);
    
#ifndef __WINDOWS_MM__
    midiOut.connect(MidiConnection::VIRTUAL);
    midiIn.connect(MidiConnection::VIRTUAL);
#endif

    midiLightsOut = make_shared<RtMidiOut>(RtMidi::Api::UNSPECIFIED, "VirtualLpp");
    midiLightsOut->setErrorCallback(handleMidiError);
    for (int i = 0; i < midiLightsOut->getPortCount(); i++)
    {
        string name = midiLightsOut->getPortName(i);
        if (midiLightsOut->getPortName(i) == "Launchpad Open Standalone Port")
        {
            midiLightsOut->openPort(i);
            break;
        }
    }
    
    midiControlIn = make_shared<RtMidiIn>(RtMidiIn::Api::UNSPECIFIED, "VirtualLpp");
    midiControlIn->setErrorCallback(handleMidiError);
    midiControlIn->setCallback(receiveMidiControlCallback, this);
    for (int i = 0; i < midiControlIn->getPortCount(); i++)
    {
        string name = midiLightsOut->getPortName(i);
        if (midiControlIn->getPortName(i) == "Launchpad Open Standalone Port")
        {
            midiControlIn->openPort(i);
            break;
        }
    }
    
    
    for (int i = 0; i <= LP_LAST_BUTTON; i++)
    {
        pads[i].setIndex(i);
    }
    
    app_init();
}

void VirtualLpp::plotLed(u8 type, u8 index, u8 red, u8 green, u8 blue)
{
    if (type == TYPESETUP)
    {
        index = LP_SETUP;
    }

    pads[index].setColor(Color(red / 255.0, green / 255.0, blue / 255.0));
    
    vector<unsigned char> msg {
        0xF0, 0x00, 0x20, 0x29,
        index, (u8)(red & 0x7F), (u8)(green & 0x7F), (u8)(blue & 0x7F),
        0xF7
    };
    midiLightsOut->sendMessage(&msg);
}

void VirtualLpp::sendMidi(u8 port, u8 status, u8 data1, u8 data2)
{
    vector<unsigned char> msg {status, data1, data2};
    midiOut.getOutPort()->sendMessage(&msg);
}

void VirtualLpp::sendSysex(u8 port, const u8* data, u16 length)
{
    
}

void VirtualLpp::receiveMidi(double timestamp, vector<unsigned char>* message, void* userData)
{
    app_midi_event(
        USBMIDI,
        (*message)[0],
        message->size() > 1 ? (*message)[1] : 0x00,
        message->size() > 2 ? (*message)[2] : 0x00);
}

void VirtualLpp::receiveMidiControl(double timestamp, vector<unsigned char>* message, void* userData)
{
    u8 type = (*message)[0] & 0xF0;
    if (type == NOTEON)
    {
        pads[(*message)[1]].press((*message)[2], false);
    }
    else if (type == NOTEOFF)
    {
        pads[(*message)[1]].press(0, false);
    }
    else if (type == POLYAFTERTOUCH)
    {
        pads[(*message)[1]].press((*message)[2], true);
    }
}

void VirtualLpp::mouseDown(MouseEvent event)
{
    int velocity = 0;
    int index = pixelToIndex(event.getX(), event.getY(), &velocity);

    mouseDown(index, velocity);
}

void VirtualLpp::mouseDown(int index, int velocity)
{
    if (index != -1)
    {
        heldIndex = index;
        heldVelocity = velocity;
        pads[index].press(velocity, false);
    }
}

void VirtualLpp::mouseUp(MouseEvent event)
{
    mouseUp();
}

void VirtualLpp::mouseUp()
{
    if (heldIndex != -1)
    {
        pads[heldIndex].press(0, false);
        heldIndex = -1;
        heldVelocity = -1;
    }
}

void VirtualLpp::mouseDrag(MouseEvent event)
{
    int velocity = 0;
    int index = pixelToIndex(event.getX(), event.getY(), &velocity);

    if (heldIndex != -1)
    {
        if (index != heldIndex)
        {
            mouseUp();
            mouseDown(index, velocity);
        }
        else if (velocity != heldVelocity)
        {
            heldVelocity = velocity;
            pads[heldIndex].press(heldVelocity, true);
        }
    }
    else
    {
        mouseDown(index, velocity);
    }
}

void VirtualLpp::update()
{
    app_timer_event();
}

void VirtualLpp::draw()
{
    double px = padPadding + padSize / 2;
    double py = lpRect.getHeight() - padSize / 2 - padPadding;

    for (int i = 0; i <= LP_LAST_BUTTON; i++)
    {
        if (i > 0 && (i % ROW_SIZE == 0))
        {
            px = padPadding + padSize / 2;
            py -= padSize + padPadding;
        }

        if (i != 0 && i != 9)
        {
            pads[i].draw(px, py, padSize, padSize);
        }

        px += padSize + padPadding;
    }
}

void VirtualLpp::setWidth(float w)
{
    lpRect.set(0, 0, w, w);
    padSize = 0.9 * w / ROW_SIZE;
    padPadding = 0.1 * w / (ROW_SIZE + 1);
}

int VirtualLpp::pixelToIndex(int px, int py, int* v)
{
    if (px < 0 || px >= lpRect.getWidth()
        || py < 0 || py > lpRect.getHeight())
    {
        return -1;
    }
    
    px -= lpRect.getX1();
    float wholePadSize = padSize + padPadding;
    int x = floor(px / wholePadSize);
    float xOffset = mod((float)px, wholePadSize);

    if (x < 0 || x >= ROW_SIZE || xOffset < padPadding)
    {
        return -1;
    }

    py = lpRect.getHeight() - (py - lpRect.getY1());
    int y = floor(py / wholePadSize);
    float yOffset = mod((float)py, wholePadSize);

    if (y < 0 || y >= ROW_SIZE || yOffset < padPadding)
    {
        return -1;
    }

    if (v != nullptr)
    {
        *v = 127 * (yOffset - padPadding) / padSize;
    }

    return y * ROW_SIZE + x;
}
