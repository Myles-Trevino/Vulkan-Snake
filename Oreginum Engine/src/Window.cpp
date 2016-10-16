#include "Core.hpp"
#include "Window.hpp"

namespace
{
	std::string title;
	glm::ivec2 resolution;
	HINSTANCE instance;
	FILE *stream;
	HWND window;
	bool resized;
	bool closed;

	LRESULT CALLBACK window_callback(HWND window, UINT message,
		WPARAM message_information, LPARAM message_informaton_long)
	{
		switch(message)
		{
		case WM_DESTROY: closed = true; break;
		default: return DefWindowProc(window, message,
			message_information, message_informaton_long);
		}
		return NULL;
	}
}

void Oreginum::Window::initialize(const std::string& title,
	const glm::ivec2& resolution, bool debug)
{
	::title = title;
	::resolution = resolution;
	::instance = GetModuleHandle(NULL);

	if(debug)
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen_s(&stream, "CONOUT$", "w", stdout);
	}

	WNDCLASSEX window_information;
	window_information.cbSize = sizeof(WNDCLASSEX);
	window_information.style = CS_HREDRAW | CS_VREDRAW;
	window_information.lpfnWndProc = window_callback;
	window_information.cbClsExtra = 0;
	window_information.cbWndExtra = 0;
	window_information.hInstance = instance;
	window_information.hIcon = LoadIcon(instance, IDI_APPLICATION);
	window_information.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_information.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	window_information.lpszMenuName = NULL;
	window_information.lpszClassName = title.c_str();
	window_information.hIconSm = LoadIcon(window_information.hInstance, IDI_APPLICATION);
	if(!RegisterClassEx(&window_information)) Core::error("Could not register window.");

	window = CreateWindow(title.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN, Core::get_screen_resolution().x/2-resolution.x/2,
		Core::get_screen_resolution().y/2-resolution.y/2, resolution.x, resolution.y,
		NULL, NULL, instance, NULL);
	if(!window) Core::error("Could not create window.");
	ShowWindow(window, SW_SHOW);
}

void Oreginum::Window::update()
{
	static MSG message;
	while(PeekMessage(&message, NULL, 0, BM_SETDONTCLICK, PM_REMOVE)) DispatchMessage(&message);
	while(PeekMessage(&message, NULL, WM_UNICHAR, NULL, PM_REMOVE)) DispatchMessage(&message);

	RECT rectangle;
	GetClientRect(window, &rectangle);
	glm::ivec2 old_resolution{resolution};
	resolution = {rectangle.right-rectangle.left, rectangle.bottom-rectangle.top};
	resized = (old_resolution != resolution);
}

HINSTANCE Oreginum::Window::get_instance(){ return instance; }

HWND Oreginum::Window::get(){ return window; }

std::string Oreginum::Window::get_title(){ return title; }

glm::ivec2 Oreginum::Window::get_resolution(){ return resolution; };

bool Oreginum::Window::was_closed(){ return closed; }

bool Oreginum::Window::was_resized(){ return resized; }

bool Oreginum::Window::is_visible(){ return (resolution.x > 0 && resolution.y > 0); }