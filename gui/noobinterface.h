/*
 * Noobinterface extension all rights goes to their respective owners
 * Thanks to the group and ocornut who made 'Dear ImGui' amazing.
 */
#ifndef _NOOBINTERFACE_H
#define _NOOBINTERFACE_H

#  define	_SDL_RENDERING_MODE	(0)
#  define	_SDL_OPENGL_RENDERING_MODE (1)

#include <imgui.h>
#include <imgui_internal.h>
#include <imconfig.h>
#include <imstb_truetype.h>
#include <imstb_textedit.h>
#include <imgui_freetype.h>
#include <imgui_sdl.h>
#include <gui/color_sel.h>

#define SET_KEY(imgui_key, key) io.KeyMap[imgui_key] = key

// Renders on OpenGL or SDL Renderer


namespace NoobInterface {
static SDL_Window *gWindow;
static SDL_Renderer *gRender;
static bool RenderMode;
static char *ClipboardTextData = NULL;
static SDL_GLContext *gContext;
static bool opened = true;
static float AverageFPS = 60.0f;
static bool ImGuiShown = true;

void (*RenderAll)(void);

void SetImGUIFPS(float FPS)
{
	if (FPS == 0) FPS = 1;
	AverageFPS = FPS;
}


void SetImGuiInterface(bool mode)
{
	ImGuiShown = mode;
}


bool IsImGuiShown(void)
{
	return ImGuiShown;
}


bool isOpened(void)
{
	return opened;
}


SDL_Renderer *NoobGetRenderer(void)
{
	return gRender;
}


SDL_Window *NoobGetWindow(void)
{
	return gWindow;
}

static const char *GetClipboardText(void *data)
{
	if (ClipboardTextData)
		SDL_free(ClipboardTextData);
	ClipboardTextData = SDL_GetClipboardText();
	return ClipboardTextData;
}


static void SetClipboardText(void *data, const char *text)
{
	SDL_SetClipboardText(text);
}


void ProcessEvents(SDL_Event *ev)
{
	int key, x, y;
	const int button_state = SDL_GetMouseState(&x, &y);

	ImGuiIO& io = ImGui::GetIO();

	switch (ev->type) {
	case SDL_QUIT: opened = false; break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:;
		key = ev->key.keysym.scancode;
		io.KeysDown[key] = ev->type == SDL_KEYDOWN;
		io.KeyShift = (SDL_GetModState() & KMOD_SHIFT) != 0;
		io.KeyAlt = (SDL_GetModState() & KMOD_ALT) != 0;
		io.KeyCtrl = (SDL_GetModState() & KMOD_CTRL) != 0;
		io.KeySuper = (SDL_GetModState() & KMOD_GUI) != 0;
		break;
	case SDL_WINDOWEVENT:
		switch (ev->window.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SDL_RenderSetLogicalSize(gRender, ev->window.data1, ev->window.data2);
			io.DisplaySize.x = (float) ev->window.data1;
			io.DisplaySize.y = (float) ev->window.data2;
			break;
		}
		break;
	case SDL_TEXTINPUT:
		io.AddInputCharactersUTF8(ev->text.text);
		break;
	case SDL_MOUSEWHEEL:
		io.MouseWheel = ev->wheel.y;
		break;
	}

	io.DeltaTime = 1.0 / AverageFPS;
	io.MousePos = ImVec2((float) x, (float) y);
	io.MouseDown[0] = button_state & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = button_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
}


void SetRenderingBlendMode(SDL_BlendMode blend_mode)
{
	IM_ASSERT(gRender);
	SDL_SetRenderDrawBlendMode(gRender, blend_mode);
}


void Init(SDL_Window *Window)
{
	gWindow = Window;
	IM_ASSERT(gWindow);

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.BackendPlatformName = "noobinterface_sdl_ext";
	SET_KEY(ImGuiKey_Tab, SDL_SCANCODE_TAB);
	SET_KEY(ImGuiKey_LeftArrow, SDL_SCANCODE_LEFT);
	SET_KEY(ImGuiKey_RightArrow, SDL_SCANCODE_RIGHT);
	SET_KEY(ImGuiKey_UpArrow, SDL_SCANCODE_UP);
	SET_KEY(ImGuiKey_DownArrow, SDL_SCANCODE_DOWN);
	SET_KEY(ImGuiKey_PageUp, SDL_SCANCODE_PAGEUP);
	SET_KEY(ImGuiKey_PageDown, SDL_SCANCODE_PAGEDOWN);
	SET_KEY(ImGuiKey_Space, SDL_SCANCODE_SPACE);
	SET_KEY(ImGuiKey_Home, SDL_SCANCODE_HOME);
	SET_KEY(ImGuiKey_End, SDL_SCANCODE_END);
	SET_KEY(ImGuiKey_Insert, SDL_SCANCODE_INSERT);
	SET_KEY(ImGuiKey_Delete, SDL_SCANCODE_DELETE);
	SET_KEY(ImGuiKey_Backspace, SDL_SCANCODE_BACKSPACE);
	SET_KEY(ImGuiKey_Enter, SDL_SCANCODE_RETURN);
	SET_KEY(ImGuiKey_Escape, SDL_SCANCODE_ESCAPE);
	SET_KEY(ImGuiKey_KeyPadEnter, SDL_SCANCODE_KP_ENTER);
	SET_KEY(ImGuiKey_A, SDL_SCANCODE_A);
	SET_KEY(ImGuiKey_C, SDL_SCANCODE_C);
	SET_KEY(ImGuiKey_V, SDL_SCANCODE_V);
	SET_KEY(ImGuiKey_X, SDL_SCANCODE_X);
	SET_KEY(ImGuiKey_Y, SDL_SCANCODE_Y);
	SET_KEY(ImGuiKey_Z, SDL_SCANCODE_Z);

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.SetClipboardTextFn = SetClipboardText;
	io.GetClipboardTextFn = GetClipboardText;
	io.ClipboardUserData = NULL;
}


void SetDrawColor(Color4i color)
{
	SDL_SetRenderDrawColor(gRender, color.r, color.g, color.b, color.a);
}


void RenderCopy(SDL_Texture *Texture, const SDL_Rect offset, const SDL_Rect scale)
{
	SDL_RenderCopy(gRender, Texture, &offset, &scale);
}


void Clear(void)
{
	SDL_RenderClear(gRender);
}


void RenderGui(void)
{
	if (ImGuiShown) {
		ImGui::Render();
		ImGuiSDL::Render(ImGui::GetDrawData());
	}
}


void RenderPresent(void)
{
	SDL_RenderPresent(gRender);
}


void SDL_Render(void)
{
	RenderGui();
	SDL_RenderPresent(gRender);
}


void OpenGL_Render(void)
{
	IM_ASSERT(0);
	SDL_GL_SwapWindow(gWindow);
}


void SetRenderingMode(void *Renderer, bool mode)
{
	int w, h;
	IM_ASSERT(RenderMode == 0);
	IM_ASSERT(gWindow != 0);
	IM_ASSERT(gRender == 0);
	IM_ASSERT(gContext == 0);

	SDL_GetWindowSize(gWindow, &w, &h);
	if (mode == _SDL_RENDERING_MODE) {
		gRender = (SDL_Renderer *) Renderer;
		IM_ASSERT(gRender);
		ImGuiSDL::Initialize(gRender, w, h);
		RenderAll = SDL_Render;
	} else if (mode == _SDL_OPENGL_RENDERING_MODE) {
		RenderAll = OpenGL_Render;
		SDL_GL_ResetAttributes();
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1); // Enable Hardware Acceleration
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
		// SDL_GL_SetSwapInterval(1); // Vsync
	} else {
		IM_ASSERT(0);
	}
}


void Destroy(void) 
{
	if (RenderMode == _SDL_RENDERING_MODE) {
		IM_ASSERT(gRender);
		ImGuiSDL::Deinitialize();
		SDL_DestroyRenderer(gRender);
		gRender = NULL;
	} else if (RenderMode == _SDL_OPENGL_RENDERING_MODE) {
		IM_ASSERT(gContext);
		//	...
	} else {
		IM_ASSERT(0);
	}

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
}
} // NoobInterface SDL Compatible

#undef SET_KEY

#endif
