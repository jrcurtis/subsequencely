// ImGui GLFW binding with OpenGL3 + shaders
// You can copy and use unmodified imgui_impl_* files in your project. 
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// See main.cpp for an example of using this.
// https://github.com/ocornut/imgui

#include <imgui.h>
#include <cinder/app/App.h>
#include <cinder/Clipboard.h>
#include <cinder/Display.h>
#include <cinder/gl/gl.h>
#include "imgui_impl_cinder_gl3.h"

// Data
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static ImVec2       g_MousePos;
static float        g_MouseWheel = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplCinder_RenderDrawLists(ImDrawData* draw_data)
{
    // Backup GL state
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
	GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
	GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
	GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup orthographic projection matrix
    const float ortho_projection[4][4] =
    {
        { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };
    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(g_VaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBindVertexArray(last_vertex_array);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(last_blend_src, last_blend_dst);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
}

static const char* ImGui_ImplCinder_GetClipboardText()
{
    return ci::Clipboard::getString().data();
}

static void ImGui_ImplCinder_SetClipboardText(const char* text)
{
    ci::Clipboard::setString(text);
}

void ImGui_ImplCinder_MouseButtonCallback(ci::app::MouseEvent e, bool isDown)
{
    if (e.isLeft())
        g_MousePressed[0] = isDown;
    else if (e.isRight())
        g_MousePressed[1] = isDown;
    else if (e.isMiddle())
        g_MousePressed[2] = isDown;
}

void ImGui_ImplCinder_MouseMoveCallback(ci::app::MouseEvent e)
{
    g_MousePos.x = e.getX();
    g_MousePos.y = e.getY();
}

void ImGui_ImplCinder_ScrollCallback(ci::app::MouseEvent e)
{
    g_MouseWheel += e.getWheelIncrement();
}

void ImGui_ImplCinder_KeyCallback(ci::app::KeyEvent e, bool isDown)
{
    ImGuiIO& io = ImGui::GetIO();
    
    io.KeysDown[e.getCode()] = isDown;

    io.KeyCtrl = io.KeysDown[ci::app::KeyEvent::KEY_LCTRL] || io.KeysDown[ci::app::KeyEvent::KEY_RCTRL];
    io.KeyShift = io.KeysDown[ci::app::KeyEvent::KEY_LSHIFT] || io.KeysDown[ci::app::KeyEvent::KEY_RSHIFT];
    io.KeyAlt = io.KeysDown[ci::app::KeyEvent::KEY_LALT] || io.KeysDown[ci::app::KeyEvent::KEY_RALT];
}

void ImGui_ImplCinder_CharCallback(ci::app::KeyEvent e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (e.getCharUtf32() > 0 && e.getCharUtf32() < 0x10000)
        io.AddInputCharacter((unsigned short)e.getCharUtf32());
}

void ImGui_ImplCinder_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

	// Create OpenGL texture
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Cleanup (don't clear the input data if you want to append new fonts later)
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();
}

bool ImGui_ImplCinder_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    g_ShaderHandle = glCreateProgram();
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    glCompileShader(g_VertHandle);
    glCompileShader(g_FragHandle);
    glAttachShader(g_ShaderHandle, g_VertHandle);
    glAttachShader(g_ShaderHandle, g_FragHandle);
    glLinkProgram(g_ShaderHandle);

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    glGenVertexArrays(1, &g_VaoHandle);
    glBindVertexArray(g_VaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glEnableVertexAttribArray(g_AttribLocationPosition);
    glEnableVertexAttribArray(g_AttribLocationUV);
    glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    ImGui_ImplCinder_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);

    return true;
}

bool    ImGui_ImplCinder_Init(bool install_callbacks)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = ci::app::KeyEvent::KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = ci::app::KeyEvent::KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = ci::app::KeyEvent::KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = ci::app::KeyEvent::KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = ci::app::KeyEvent::KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = ci::app::KeyEvent::KEY_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = ci::app::KeyEvent::KEY_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = ci::app::KeyEvent::KEY_HOME;
    io.KeyMap[ImGuiKey_End] = ci::app::KeyEvent::KEY_END;
    io.KeyMap[ImGuiKey_Delete] = ci::app::KeyEvent::KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = ci::app::KeyEvent::KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = ci::app::KeyEvent::KEY_RETURN;
    io.KeyMap[ImGuiKey_Escape] = ci::app::KeyEvent::KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = ci::app::KeyEvent::KEY_a;
    io.KeyMap[ImGuiKey_C] = ci::app::KeyEvent::KEY_c;
    io.KeyMap[ImGuiKey_V] = ci::app::KeyEvent::KEY_v;
    io.KeyMap[ImGuiKey_X] = ci::app::KeyEvent::KEY_x;
    io.KeyMap[ImGuiKey_Y] = ci::app::KeyEvent::KEY_y;
    io.KeyMap[ImGuiKey_Z] = ci::app::KeyEvent::KEY_z;

    io.RenderDrawListsFn = ImGui_ImplCinder_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = ImGui_ImplCinder_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplCinder_GetClipboardText;

    if (install_callbacks)
    {
        ci::app::AppBase* app = ci::app::AppBase::get();
        ci::app::WindowRef window = app->getWindow();
        
        window->getSignalMouseDown().connect([] (ci::app::MouseEvent e) {
            ImGui_ImplCinder_MouseButtonCallback(e, true);
        });
        window->getSignalMouseUp().connect([] (ci::app::MouseEvent e) {
            ImGui_ImplCinder_MouseButtonCallback(e, false);
        });
        window->getSignalMouseMove().connect([] (ci::app::MouseEvent e) {
            ImGui_ImplCinder_MouseMoveCallback(e);
        });
        window->getSignalMouseDrag().connect([] (ci::app::MouseEvent e) {
            ImGui_ImplCinder_MouseMoveCallback(e);
        });
        window->getSignalMouseWheel().connect([] (ci::app::MouseEvent e) {
            ImGui_ImplCinder_ScrollCallback(e);
        });
        window->getSignalKeyDown().connect([] (ci::app::KeyEvent e) {
            ImGui_ImplCinder_KeyCallback(e, true);
            ImGui_ImplCinder_CharCallback(e);
        });
        window->getSignalKeyUp().connect([] (ci::app::KeyEvent e) {
            ImGui_ImplCinder_KeyCallback(e, false);
        });
    }

    return true;
}

void ImGui_ImplCinder_Shutdown()
{
    if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
    if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
    g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

    glDetachShader(g_ShaderHandle, g_VertHandle);
    glDeleteShader(g_VertHandle);
    g_VertHandle = 0;

    glDetachShader(g_ShaderHandle, g_FragHandle);
    glDeleteShader(g_FragHandle);
    g_FragHandle = 0;

    glDeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
    ImGui::Shutdown();
}

void ImGui_ImplCinder_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplCinder_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    
    ci::app::AppBase* app = ci::app::AppBase::get();
    w = app->getWindowWidth();
    h = app->getWindowHeight();
    display_w = w;
    display_h = h;
    
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time =  app->getElapsedSeconds();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (app->getForegroundWindow() != nullptr)
    {
    	io.MousePos = ImVec2(g_MousePos);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
    	io.MousePos = ImVec2(-1,-1);
    }

    for (int i = 0; i < 3; i++)
    {
        io.MouseDown[i] = g_MousePressed[i];    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        //g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
//    if (io.MouseDrawCursor)
//    {
//        app->showCursor();
//    }
//    else
//    {
//        app->hideCursor();
//    }

    // Start the frame
    ImGui::NewFrame();
}
