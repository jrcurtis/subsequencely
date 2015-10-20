
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
    
private:
    void drawGui();
    void drawBottomPanel();
    void drawSidePanel();
    void drawSequenceNotes(Sequence& s);
    void drawSequenceInfo(Sequence& s);
    
    VirtualLpp lpp;
    lpp::Timer lppTimer;
    mutex lppMutex;
    
    ImGuiIO& io;
    bool showGui;
    ImVec2 sidePanelPos;
    ImVec2 sidePanelSize;
    ImVec2 bottomPanelPos;
    ImVec2 bottomPanelSize;
    
    array<string, GRID_SIZE> sequenceNames;
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
    
    for (int i = 0; i < GRID_SIZE; i++)
    {
        sequenceNames[i] = "Sequence " + to_string(i + 1);
    }
    
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
}

void VirtualLppApp::mouseUp(MouseEvent event)
{
    lock_guard<mutex> lock(lppMutex);
    lpp.mouseUp(event);
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
    drawBottomPanel();
    drawSidePanel();
}

void VirtualLppApp::drawBottomPanel()
{
    ImGui::SetNextWindowPos(bottomPanelPos);
    ImGui::SetNextWindowSize(bottomPanelSize);
    ImGui::Begin(
                 "bottom panel", NULL,
                 windowFlags | ImGuiWindowFlags_HorizontalScrollbar);
    
    string scaleSteps = "";
    int lastOffset = 0;
    for (int i = 1; i < sequencer.scale.num_notes; i++)
    {
        scaleSteps += to_string(sequencer.scale.offsets[i] - lastOffset);
        lastOffset = sequencer.scale.offsets[i];
        scaleSteps += ", ";
    }
    scaleSteps += to_string(12 - lastOffset);
    
    ImGui::Value("BPM", khz_to_bpm((float)sequencer.tempo));
    ImGui::SameLine();
    ImGui::Text("Scale: %s", scaleSteps.data());
    ImGui::SameLine();
    ImGui::Text("Modifiers: 0x%08x", (unsigned int)modifiers);
    
    for (int seqI = 0; seqI < GRID_SIZE; seqI++)
    {
        drawSequenceNotes(sequencer.sequences[seqI]);
    }
    
    ImGui::End();
}

void VirtualLppApp::drawSidePanel()
{
    ImGui::SetNextWindowPos(sidePanelPos);
    ImGui::SetNextWindowSize(sidePanelSize);
    ImGui::Begin("side panel", NULL, windowFlags);
    
    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (ImGui::TreeNode(sequenceNames[i].data()))
        {
            ImGui::BeginGroup();
            drawSequenceInfo(sequencer.sequences[i]);
            ImGui::EndGroup();
            ImGui::TreePop();
        }
    }
    
    ImGui::End();
}

void VirtualLppApp::drawSequenceNotes(Sequence& s)
{
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

void VirtualLppApp::drawSequenceInfo(Sequence& s)
{
    ImGui::Text("State: %s",
        flag_is_set(s.flags, SEQ_PLAYING)
            ? "Playing" : "Stopped");
    
    ImGui::Value("Muted", flag_is_set(s.flags, SEQ_MUTED) != 0);
    ImGui::Value("Soloed", flag_is_set(s.flags, SEQ_SOLOED) != 0);
    ImGui::Value("Armed", flag_is_set(s.flags, SEQ_ARMED) != 0);

    ImGui::Value("Octave", s.layout.octave);
    ImGui::Value("Root Note", s.layout.root_note);
    ImGui::Value("Channel", s.channel);
    
    ImGui::Value("Record Control", flag_is_set(s.flags, SEQ_RECORD_CONTROL) != 0);
    ImGui::Value("Control Code", s.control_code);
}


CINDER_APP(VirtualLppApp,
           RendererGl(RendererGl::Options().msaa(4)),
           [&]( App::Settings *settings ) {
               settings->setWindowSize(defaultSize, defaultSize);
           })




