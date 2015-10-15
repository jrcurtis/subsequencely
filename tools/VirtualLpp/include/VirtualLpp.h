
#pragma once

#include <string>

#include "cinder/Cinder.h"
#include "cinder/cairo/Cairo.h"

#include "app.h"
#include "seq.h"
#include "VirtualPad.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace cairo;
using namespace glm;


class VirtualLpp
{
public:
    VirtualLpp(int width=300);

    static VirtualLpp* getInstance()
    {
        return VirtualLpp::instance;
    }

    void plotLed(u8 type, u8 index, u8 red, u8 green, u8 blue);
    void sendMidi(u8 port, u8 status, u8 data1, u8 data2);
    void sendSysex(u8 port, const u8* data, u16 length);

    void update();
    void draw(Context& ctx);
    void setWidth(double w);

    void mouseDown(MouseEvent event);
    void mouseDown(int index, int velocity);
    void mouseUp(MouseEvent event);
    void mouseUp();
    void mouseDrag(MouseEvent event);

private:
    static VirtualLpp* instance;

    int pixelToIndex(int x, int y, int* v=nullptr);

    Rectd lpRect;
    double padSize;
    double padPadding;
    int heldIndex;
    int heldVelocity;
    VirtualPad pads[LP_LAST_BUTTON + 1];
};
