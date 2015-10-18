// ImGui Cinder binding with OpenGL3 + shaders
// Adapted from the GLFW example code
// You can copy and use unmodified imgui_impl_* files in your project. 
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// See main.cpp for an example of using this.
// https://github.com/ocornut/imgui

#include <cinder/app/MouseEvent.h>

IMGUI_API bool        ImGui_ImplCinder_Init(bool install_callbacks);
IMGUI_API void        ImGui_ImplCinder_Shutdown();
IMGUI_API void        ImGui_ImplCinder_NewFrame();

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplCinder_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplCinder_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
IMGUI_API void        ImGui_ImplCinder_MouseButtonCallback(ci::app::MouseEvent e, bool isDown);
IMGUI_API void        ImGui_ImplCinder_ScrollCallback(ci::app::MouseEvent e);
IMGUI_API void        ImGui_ImplCinder_KeyCallback(ci::app::KeyEvent e, bool isDown);
IMGUI_API void        ImGui_ImplCinder_CharCallback(ci::app::KeyEvent e);