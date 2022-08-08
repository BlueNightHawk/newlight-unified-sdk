#include "cl_dll.h"
#include "PlatformHeaders.h"
#include <Psapi.h>
#include "FontAwesome.h"
#include "fs_aux.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../external/imgui/stb_image.h"

#include "hl_imgui.h"
#include "CChapterSelectGUI.h"

extern cl_enginefunc_t gEngfuncs;
extern int isPaused;
extern bool inMainMenu;
SDL_Window *window = NULL;
SDL_GLContext gl_context = NULL;

// To draw imgui on top of Half-Life, we take a detour from certain engine's function into HL_ImGUI_Draw function
void HL_ImGUI_Init() {

	// One of the final steps before drawing a frame is calling SDL_GL_SwapWindow function
	// It must be prevented, so imgui could be drawn before calling SDL_GL_SwapWindow

	// This will hold the constant address of x86 CALL command, which looks like this
	// E8 FF FF FF FF
	// Last 4 bytes specify an offset from this address + 5 bytes of command itself
	unsigned int origin = NULL;

	// We're scanning 1 MB at the beginning of hw.dll for a certain sequence of bytes
	// Based on location of that sequnce, the location of CALL command is calculated
	MODULEINFO module_info;
	if ( GetModuleInformation( GetCurrentProcess(), GetModuleHandle( "hw.dll" ), &module_info, sizeof( module_info ) ) ) {
		origin = ( unsigned int ) module_info.lpBaseOfDll;
		
		const int MEGABYTE = 1024 * 1024;
		char *slice = new char[MEGABYTE];
		ReadProcessMemory( GetCurrentProcess(), ( const void * ) origin, slice, MEGABYTE, NULL );

		unsigned char magic[] = { 0x8B, 0x4D, 0x08, 0x83, 0xC4, 0x08, 0x89, 0x01, 0x5D, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0xA1 };

		for ( unsigned int i = 0 ; i < MEGABYTE - 16; i++ ) {
			bool sequenceIsMatching = memcmp( slice + i, magic, 16 ) == 0;
			if ( sequenceIsMatching ) {
				origin += i + 27;
				break;
			}
		}

		delete[] slice;

		char opCode[1];
		ReadProcessMemory( GetCurrentProcess(), ( const void * ) origin, opCode, 1, NULL );
		if ( opCode[0] != 0xFFFFFFE8 ) {
			gEngfuncs.Con_DPrintf( "Failed to embed ImGUI: expected CALL OP CODE, but it wasn't there\n" );
			return;
		}
	} else {
		gEngfuncs.Con_DPrintf( "Failed to embed ImGUI: failed to get hw.dll memory base address\n" );
		return;
	}
	// Setup window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	window = SDL_GetWindowFromID( 1 );
	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL2_Init();

	// To make a detour, an offset to dedicated function must be calculated and then correctly replaced
	unsigned int detourFunctionAddress = ( unsigned int ) &HL_ImGUI_Draw;
	unsigned int offset = ( detourFunctionAddress ) - origin - 5;

	// The resulting offset must be little endian, so 
	// 0x0A852BA1 => A1 2B 85 0A
	char offsetBytes[4];
	for ( int i = 0; i < 4; i++ ) {
		offsetBytes[i] = ( offset >> ( i * 8 ) );
	}

	// This is WinAPI call, blatantly overwriting the memory with raw pointer would crash the program
	// Notice the 1 byte offset from the origin
	WriteProcessMemory( GetCurrentProcess(), ( void * ) ( origin + 1 ), offsetBytes, 4, NULL );

	SDL_AddEventWatch( HL_ImGUI_ProcessEvent, NULL );

	ImGuiStyle *style = &ImGui::GetStyle();
	style->WindowRounding = 0.0f;
	style->ScrollbarRounding = 0.0f;

	style->Colors[ImGuiCol_PopupBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.86f);
	style->Colors[ImGuiCol_TitleBg]               = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.20f, 0.20f, 0.60f);
	style->Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
	style->Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
	style->Colors[ImGuiCol_Button]                = ImVec4(0.63f, 0.63f, 0.63f, 0.60f);
	style->Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.63f, 0.63f, 0.63f, 0.71f);
	style->Colors[ImGuiCol_ButtonActive]          = ImVec4(0.51f, 0.51f, 0.51f, 0.60f);
	style->Colors[ImGuiCol_Header]                = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive]          = ImVec4(0.53f, 0.53f, 0.53f, 0.51f);
	style->Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.53f, 0.53f, 0.53f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogram]         = ImVec4(1.00f, 1.00f, 1.00f, 0.35f);

	style->WindowPadding = ImVec2( 8.0f, 4.0f );

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF( FS_ResolveModPath( "resource\\DroidSans.ttf" ).c_str(), 16 );
	ImFontConfig config;
	config.MergeMode = true;
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF( FS_ResolveModPath( "resource\\fontawesome-webfont.ttf" ).c_str(), 14.0f, &config, icon_ranges );
	static const ImWchar icon_ranges_cyrillic[] = { 0x0410, 0x044F, 0 };
	io.Fonts->AddFontFromFileTTF( FS_ResolveModPath( "resource\\DroidSans.ttf" ).c_str(), 16, &config, icon_ranges_cyrillic );

	g_ChapterSelectGUI.Init();
}

void HL_ImGUI_Deinit() {
	if ( !window ) {
		return;
	}
	g_ChapterSelectGUI.DeleteImageTextures();
	SDL_DelEventWatch( HL_ImGUI_ProcessEvent, NULL );
}

extern cvar_t  *printmodelindexes;
extern cvar_t  *print_aim_entity;
extern cvar_t  *print_player_info;
void HL_ImGUI_Draw() {

    ImGuiIO& io = ImGui::GetIO();

    // Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	if ( gHUD.m_bIsPaused || gHUD.m_bInMainMenu ) {
		g_ChapterSelectGUI.Draw();

		SDL_ShowCursor( 1 );
	} else {
		SDL_ShowCursor( 0 );
	}

    // Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	// glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

int HL_ImGUI_ProcessEvent( void *data, SDL_Event* event ) {
	return ImGui_ImplSDL2_ProcessEvent( event );
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}