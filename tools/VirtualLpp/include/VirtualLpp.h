
#pragma once

#include <string>
#include <memory>
#include <array>

#include "cinder/Cinder.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "RtMidi.h"

#include "MidiConnection.h"
#include "seq.h"
#include "VirtualPad.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace glm;


class VirtualLpp
{
public:
    static const char* VIRTUAL_IN_NAME;
    static const char* VIRTUAL_OUT_NAME;

    VirtualLpp(int width=300);
    ~VirtualLpp();

    static VirtualLpp* getInstance()
    {
        return VirtualLpp::instance;
    }

    void plotLed(uint8_t type, uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
    void sendMidi(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2);
    void sendSysex(uint8_t port, const uint8_t* data, uint16_t length);
    void readFlash(uint32_t offset, uint8_t* data, uint32_t length);
    void writeFlash(uint32_t offset, const uint8_t* data, uint32_t length);

    void update();
    void draw();
    void setWidth(float w);
    
    void receiveMidi(double timestamp, vector<unsigned char>* message, void* userData);
    void receiveMidiControl(double timestamp, vector<unsigned char>* message, void* userData);

    void mouseDown(MouseEvent event);
    void mouseDown(int index, int velocity);
    void mouseUp(MouseEvent event);
    void mouseUp();
    void mouseDrag(MouseEvent event);

    MidiConnection& getMidiOut() { return midiOut; }
    MidiConnection& getMidiIn() { return midiIn; }

private:

    static VirtualLpp* instance;

    int pixelToIndex(int x, int y, int* v=nullptr);

    void loadUserArea();
    void saveUserArea();

    Rectd lpRect;
    float padSize;
    float padPadding;
    
    int heldIndex;
    int heldVelocity;
    
    MidiConnection midiOut;
    MidiConnection midiIn;
    
    shared_ptr<RtMidiOut> midiLightsOut;
    shared_ptr<RtMidiIn> midiControlIn;
    
    array<VirtualPad, LP_LAST_BUTTON + 1> pads;
    array<uint8_t, USER_AREA_SIZE> userArea;
};
