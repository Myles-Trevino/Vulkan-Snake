#include "Error.hpp"
#include "Window.hpp"

LRESULT CALLBACK Window::window_callback(HWND window, UINT message,
	WPARAM information, LPARAM informaton_long)
{
	switch(message)
	{
	case WM_CLOSE: DestroyWindow(window); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	case WM_PAINT: ValidateRect(window, NULL); break;
	default: return DefWindowProc(window, message, information, informaton_long);
	}
	return NULL;
}

Window::Window(const std::string& title, const glm::ivec2& resolution,
	HINSTANCE instance, bool debug)
	: TITLE(title), resolution(resolution), instance(instance)
{
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
	if(!RegisterClassEx(&window_information)) error("Could not register window.");

	screen_resolution = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	window = CreateWindow(title.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN, screen_resolution.x/2-resolution.x/2, screen_resolution.y/2-
		resolution.y/2, resolution.x, resolution.y, NULL, NULL, instance, NULL);
	if(!window) error("Could not create window.");
	ShowWindow(window, SW_SHOW);
}

void Window::update()
{
	static MSG message;
	if(!GetMessage(&message, NULL, 0, 0)) exited = true;
	TranslateMessage(&message);
	DispatchMessage(&message);
}