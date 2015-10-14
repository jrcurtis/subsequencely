
#include "VirtualPad.h"

Color VirtualPad::defaultColor = Color(0.7, 0.7, 0.7);
FontFace VirtualPad::font("Arial");

void VirtualPad::draw(Context& ctx, double x, double y, double w, double h)
{
    double r = w / 2;
    double sizeScale = label == "Setup" ? 0.5 : 1.0;
    GradientRadial grad(x + r, y + r, 0, x + r, y + r, 1.5 * r);
    grad.addColorStop(0, color);
    grad.addColorStop(r * brightness, defaultColor);
    
    ctx.setSource(grad);
    ctx.newSubPath();
    
    if (isButton)
    {
        ctx.translate(r, r);
        ctx.arc(x, y, r * sizeScale, 0.0, 2 * M_PI);
        ctx.translate(-r, -r);
        // draw label
    }
    else
    {
        ctx.rectangle(x, y, w, h);
    }
    
    ctx.closePath();
    ctx.fill();
    
    if (isButton)
    {
        ctx.setSource(Color::black());
        ctx.setFontFace(font);
        double fontSize = sizeScale * w / 4;
        ctx.setFontSize(fontSize);
        
        TextExtents te = ctx.textExtents(label);
        double tx = x + r - te.width() / 2;
        double ty = y + fontSize / 2 + r - te.height() / 2;
        ctx.translate(tx, ty);
        ctx.textPath(label);
        
        ctx.fill();
        ctx.translate(-tx, -ty);
    }
}
