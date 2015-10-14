
#include "cinder/app/App.h"
#include "cinder/Path2d.h"
#include "cinder/cairo/Cairo.h"
#include "cinder/ip/Fill.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"

#include "VirtualLpp.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace glm;

class VirtualLppApp : public App
{
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void mouseUp(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void resize() override;
    void update() override;
    void draw() override;
    void renderScene( cairo::Context &ctx );
    
private:
    VirtualLpp lpp;
};

void VirtualLppApp::setup()
{
    lpp.setWidth(800);
}

void VirtualLppApp::mouseDown(MouseEvent event)
{
    lpp.mouseDown(event);
}

void VirtualLppApp::mouseUp(MouseEvent event)
{
    lpp.mouseUp(event);
}

void VirtualLppApp::mouseDrag(MouseEvent event)
{
    lpp.mouseDrag(event);
}

void VirtualLppApp::resize()
{
    int w = min(getWindowWidth(), getWindowHeight());
    lpp.setWidth(w);
    setWindowSize(w, w);
}

void VirtualLppApp::update()
{
    for (int i = 0; i < 1000 / (int)getFrameRate(); i++)
    {
        lpp.update();
    }
}

void VirtualLppApp::draw()
{
    cairo::Context ctx( cairo::createWindowSurface() );
    renderScene( ctx );
}

void VirtualLppApp::renderScene( cairo::Context &ctx )
{
    // clear the context with our radial gradient
    cairo::GradientRadial radialGrad( getWindowCenter(), 0, getWindowCenter(), getWindowWidth() );
    radialGrad.addColorStop( 0, Color( 1, 1, 1 ) );
    radialGrad.addColorStop( 1, Color( 0.6, 0.6, 0.6 ) );
    ctx.setSource( radialGrad );
    ctx.paint();
    
    lpp.draw(ctx);
}

CINDER_APP( VirtualLppApp, Renderer2d, [&]( App::Settings *settings ) {
        settings->setWindowSize( 800, 800 );
    } )
