
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
    : label(l),
    isButton(l != ""),
    color(defaultColor)
    { }
    
    VirtualPad(const char* l=nullptr)
    : label(l == nullptr ? "" : l),
    isButton(l != nullptr),
    color(defaultColor)
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
    
private:
    const string label;
    bool isButton;
    Color color;
    float brightness;
};
