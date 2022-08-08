#ifndef HL_IMGUI_H
#define HL_IMGUI_H

#include "SDL2\SDL.h"
#include "SDL2\SDL_opengl.h"
#include "../../../external/imgui/imgui.h"
#include "../../../external/imgui/imgui_impl_sdl.h"
#include "../../../external/imgui/imgui_impl_opengl2.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

void HL_ImGUI_Init();
void HL_ImGUI_Deinit();
void HL_ImGUI_Draw();
int HL_ImGUI_ProcessEvent( void *data, SDL_Event* event );

#endif // HL_IMGUI_H