
#include <iostream>
#include <glm/glm.hpp>

#include "VirtualLpp.h"

using namespace glm;
using namespace std::placeholders;

void handleMidiError(RtMidiError::Type type, const string& errorText)
{
    cout << "midi error: " << errorText << endl;
}

void receiveMidiCallback(double timestamp, vector<unsigned char>* message, void* userData)
{
    ((VirtualLpp*)userData)->receiveMidi(timestamp, message, nullptr);
}

VirtualLpp* VirtualLpp::instance = nullptr;

VirtualLpp::VirtualLpp(int width)
    : heldIndex(-1),
      heldVelocity(-1),
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
    
    try
    {
        midiOut = make_shared<RtMidiOut>(RtMidi::Api::UNSPECIFIED, "VirtualLpp");
        midiOut->setErrorCallback(handleMidiError);
        midiOut->openVirtualPort("VirtualLppOut");
    }
    catch (RtMidiError& error)
    {
        //cout << "midi error: " << error.getMessage() << endl;
    }
    
    try
    {
        midiIn = make_shared<RtMidiIn>(RtMidi::Api::UNSPECIFIED, "VirtualLpp");
        midiIn->setErrorCallback(handleMidiError);
        midiIn->setCallback(receiveMidiCallback, this);
        midiIn->openVirtualPort("VirtualLppIn");
    }
    catch (RtMidiError& error)
    {
        
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
}

void VirtualLpp::sendMidi(u8 port, u8 status, u8 data1, u8 data2)
{
    vector<unsigned char> bytes {status, data1, data2};
    midiOut->sendMessage(&bytes);
}

void VirtualLpp::sendSysex(u8 port, const u8* data, u16 length)
{
    
}

void VirtualLpp::receiveMidi(double timestamp, vector<unsigned char>* message, void* userData)
{
    u8 type = (*message)[0] & 0xF0;
    if (type == NOTEON || type == POLYAFTERTOUCH)
    {
        pads[(*message)[1]].press((*message)[2]);
    }
    else if (type == NOTEOFF)
    {
        pads[(*message)[1]].press(0);
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
        pads[index].press(velocity);
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
        pads[heldIndex].press(0);
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
            pads[heldIndex].press(heldVelocity);
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

void VirtualLpp::draw(Context& ctx)
{
    ctx.setSource(Color(0.2, 0.2, 0.2));
    ctx.moveTo(lpRect.x1, lpRect.x2);

    ctx.rectangle(0, 0, lpRect.getWidth(), lpRect.getHeight());
    
    ctx.fill();

    double px = padPadding;
    double py = lpRect.getHeight() - padSize - padPadding;

    for (int i = 0; i <= LP_LAST_BUTTON; i++)
    {
        if (i > 0 && (i % ROW_SIZE == 0))
        {
            px = padPadding;
            py -= padSize + padPadding;
        }

        if (i != 0 && i != 9)
        {
            pads[i].draw(ctx, px, py, padSize, padSize);
        }

        px += padSize + padPadding;
    }
}

void VirtualLpp::setWidth(double w)
{
    lpRect.set(0, 0, w, w);
    padSize = 0.9 * w / ROW_SIZE;
    padPadding = 0.1 * w / (ROW_SIZE + 1);
}

int VirtualLpp::pixelToIndex(int px, int py, int* v)
{
    px -= lpRect.getX1();
    double wholePadSize = padSize + padPadding;
    int x = floor(px / wholePadSize);
    double xOffset = mod((double)px, wholePadSize);

    if (x < 0 || x >= ROW_SIZE || xOffset < padPadding)
    {
        return -1;
    }

    py = lpRect.getHeight() - (py - lpRect.getY1());
    int y = floor(py / wholePadSize);
    double yOffset = mod((double)py, wholePadSize);

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
