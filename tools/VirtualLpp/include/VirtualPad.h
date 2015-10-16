
#pragma once

#include <string>

#include "cinder/Cinder.h"
#include "cinder/cairo/Cairo.h"
#include "glm/gtx/component_wise.hpp"

#include "app.h"
#include "seq.h"

using namespace std;
using namespace ci;
using namespace cairo;
using namespace glm;

class VirtualPad
{
public:
    static Color defaultColor;
    static FontFace font;
    
    VirtualPad(string l="")
    : index(0),
      label(l),
      isButton(l != ""),
      color(defaultColor),
      held(false),
      velocity(0)
    { }
    
    VirtualPad(const char* l=nullptr)
    : index(0),
      label(l == nullptr ? "" : l),
      isButton(l != nullptr),
      color(defaultColor),
      held(false),
      velocity(0)
    { }
    
    void draw(Context& ctx, double x, double y, double w, double h);
    
    Color getColor() { return color; }
    void setColor(Color c)
    {
        vec3 v = c;
        float m = compMax(v);
        brightness = compAdd(v) / 3;
        float scale = m == 0 ? 0 : 1.0 / m;
        color = scale == 0 ? defaultColor : scale * c;
    }
    
    void press(u8 v)
    {
        if (held && v > 0)
        {
            if (!isButton)
            {
                app_aftertouch_event(index, v);
            }
        }
        else
        {
            app_surface_event(
                index == LP_SETUP ? TYPESETUP : TYPEPAD,
                index,
                v);
        }
        
        held = v > 0;
        velocity = v;
    }
    
    u8 getIndex() { return index; }
    void setIndex(u8 i) { index = i; }
    
    bool isHeld() { return held; }
    
    int getVelocity() { return velocity; }
    
private:
    u8 index;
    
    const string label;
    bool isButton;
    Color color;
    float brightness;
    
    bool held;
    u8 velocity;
};
