
#include <thread>
#include <utility>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ip/Fill.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"

#include "imgui.h"
#include "imgui_impl_cinder_gl3.h"

#include "VirtualLpp.h"
#include "Timer.h"

using namespace ci;
using namespace ci::app;

using namespace std;
using namespace std::chrono;

using namespace glm;

extern Sequencer sequencer;

const int defaultSize = 800;

const ImGuiWindowFlags windowFlags =
    ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoCollapse;

class VirtualLppApp : public App
{
public:
    VirtualLppApp()
        : io(ImGui::GetIO())
    { }
    
    void setup() override;
    void cleanup() override;
    
    void mouseDown( MouseEvent event ) override;
    void mouseUp(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    
    void resize() override;
    void update() override;
    void draw() override;
    void drawGui();
    
private:
    VirtualLpp lpp;
    lpp::Timer lppTimer;
    mutex lppMutex;
    
    ImGuiIO& io;
    bool showGui;
    ImVec2 sidePanelPos;
    ImVec2 sidePanelSize;
    ImVec2 bottomPanelPos;
    ImVec2 bottomPanelSize;
};

void VirtualLppApp::setup()
{
    lpp.setWidth(defaultSize);
    lppTimer.start(milliseconds(1), [&] {
        lock_guard<mutex> lock(lppMutex);
        lpp.update();
    });
    
    ImGui_ImplCinder_Init(true);
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        getAssetPath("Cousine-Regular.ttf").string().data(),
        14.0);
    io.Fonts->Build();
    
    showGui = true;
    
    gl::enableAlphaBlending();
}

void VirtualLppApp::cleanup()
{
    lppTimer.stop();
    ImGui_ImplCinder_Shutdown();
}

void VirtualLppApp::mouseDown(MouseEvent event)
{
    lock_guard<mutex> lock(lppMutex);
    lpp.mouseDown(event);
    io.MouseDown[0] = true;
    io.MousePos = ImVec2(event.getX(), event.getY());
}

void VirtualLppApp::mouseUp(MouseEvent event)
{
    lock_guard<mutex> lock(lppMutex);
    lpp.mouseUp(event);
    io.MouseDown[0] = false;
}

void VirtualLppApp::mouseDrag(MouseEvent event)
{
    lock_guard<mutex> lock(lppMutex);
    lpp.mouseDrag(event);
}

void VirtualLppApp::keyDown(KeyEvent event)
{
    if (event.getChar() == '`')
    {
        showGui = !showGui;
        resize();
    }
}

void VirtualLppApp::keyUp(KeyEvent event)
{
    
}

void VirtualLppApp::resize()
{
    int w = getWindowWidth();
    int h = getWindowHeight();
    
    int lppSize = min(getWindowWidth(), getWindowHeight());
    
    if (showGui)
    {
        lppSize = 2 * lppSize / 3;
        sidePanelPos = ImVec2(lppSize, 0);
        sidePanelSize = ImVec2(w - lppSize, lppSize);
        bottomPanelPos = ImVec2(0, lppSize);
        bottomPanelSize = ImVec2(w, h - lppSize);
    }
    
    {
        lock_guard<mutex> lock(lppMutex);
        lpp.setWidth(lppSize);
    }
    gl::setMatricesWindow(w, h);
}

void VirtualLppApp::update()
{
    
}

void VirtualLppApp::draw()
{
    gl::setMatricesWindow(getWindowSize());
    gl::clear(ColorAf(0.2, 0.2, 0.2));
    lpp.draw();
    
    ImGui_ImplCinder_NewFrame();
    if (showGui)
    {
        drawGui();
    }
    ImGui::Render();
}

void VirtualLppApp::drawGui()
{
/*******************************************************************************
 * Side Panel
 ******************************************************************************/
    ImGui::SetNextWindowPos(sidePanelPos);
    ImGui::SetNextWindowSize(sidePanelSize);
    ImGui::Begin("side panel", NULL, windowFlags);
    
    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (flag_is_set(sequencer.sequences[i].flags, SEQ_PLAYING))
        {
            ImGui::Text("It's playing!");
        }
        else
        {
            ImGui::Text("It isn't playing :(");
        }
    }
    
    ImGui::End();

/*******************************************************************************
 * Bottom Panel
 ******************************************************************************/
    ImGui::SetNextWindowPos(bottomPanelPos);
    ImGui::SetNextWindowSize(bottomPanelSize);
    ImGui::Begin(
        "bottom panel", NULL,
        windowFlags | ImGuiWindowFlags_HorizontalScrollbar);
    
    ImGui::PushItemWidth(50);
    ImGui::LabelText("BPM", "%.2f", khz_to_bpm((float)sequencer.tempo));
    
    for (int seqI = 0; seqI < GRID_SIZE; seqI++)
    {
        Sequence& s = sequencer.sequences[seqI];
        
        for (int stepI = 0; stepI < SEQUENCE_LENGTH; stepI++)
        {
            Note& n = s.notes[stepI];
            
            ImVec4 color =
                s.playhead == stepI ? ImVec4(0.2, 0.2, 0.2, 1)
                : flag_is_set(n.flags, NTE_SLIDE) ? ImVec4(0, 0, 0.3, 1)
                : n.note_number > -1 ? ImVec4(0, 0.3, 0, 1)
                : ImVec4(0.2, 0, 0, 1);
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            
            ImGui::SmallButton(to_string(n.note_number).data());
            
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::Dummy(ImVec2(0, 0));
    }
    
    ImGui::End();
}

CINDER_APP(VirtualLppApp,
           RendererGl(RendererGl::Options().msaa(4)),
           [&]( App::Settings *settings ) {
               settings->setWindowSize(defaultSize, defaultSize);
           })




