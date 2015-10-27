
#include "VirtualPad.h"

Color VirtualPad::defaultColor = Color(0.7, 0.7, 0.7);
Font VirtualPad::font;
gl::TextureFontRef VirtualPad::textureFont;
gl::GlslProgRef VirtualPad::prog;
gl::BatchRef VirtualPad::rectBatch;
gl::BatchRef VirtualPad::circleBatch;

VirtualPad::VirtualPad(const char* l)
    : index(0),
      label(l == nullptr ? "" : l),
      isButton(l != nullptr),
      color(defaultColor),
      held(false),
      velocity(0)
{
    if (prog == nullptr)
    {
        prog = gl::GlslProg::create(
            app::loadAsset("shaders/pad.v.glsl"),
            app::loadAsset("shaders/pad.f.glsl"));
        rectBatch = gl::Batch::create(
            geom::RoundedRect(Rectf(-0.5, -0.5, 0.5, 0.5), 0.2)
                .cornerSubdivisions(4),
            prog);
        circleBatch = gl::Batch::create(
            geom::Circle().radius(0.5).subdivisions(20),
            prog);
        font = Font("Comic Sans MS", 24);
        textureFont = gl::TextureFont::create(font);
    }
}

void VirtualPad::press(u8 v, bool aftertouch)
{
    if (aftertouch)
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
    
    held = aftertouch || v != 0;
    velocity = v;
}

void VirtualPad::draw(float x, float y, float w, float h)
{
    float sizeScale = 1.0;
    float labelOffset = 0.0;
    ColorAf fontColor = ColorAf(0, 0, 0, 1);
    
    if (index == LP_SETUP)
    {
        sizeScale = 0.5;
        labelOffset = -h / 3;
        fontColor = ColorAf(1, 1, 1, 1);
    }
    
    gl::pushModelMatrix();
    gl::translate(x, y);
    gl::scale(w * sizeScale, h * sizeScale, 1);
    prog->uniform("uColor", color.get(ColorModel::CM_RGB));
    
    if (isButton)
    {
        circleBatch->draw();
    }
    else
    {
        rectBatch->draw();
    }
    
    gl::popModelMatrix();
    
    if (isButton)
    {
        float fontScale = h / 4 / font.getSize();
        auto opts = gl::TextureFont::DrawOptions().scale(fontScale);
        vec2 size = textureFont->measureString(label, opts);
        gl::color(fontColor);
        textureFont->drawString(
            label,
            vec2(x - fontScale * size.x / 2,
                 y + fontScale * size.y / 4 + labelOffset),
            opts);
    }

}
