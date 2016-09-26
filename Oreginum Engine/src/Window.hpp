#pragma once
#include <string>
#include <windows.h>
#include <GLM/glm.hpp>

class Window
{
public:
	Window(const std::string& title, const glm::ivec2& resolution,
		HINSTANCE instance, bool debug = false);

	void update();

	HINSTANCE get_instance() const { return instance; }
	HWND get() const { return window; }
	std::string get_title() const { return TITLE; }
	glm::ivec2 get_resolution() const { return resolution; };
	bool was_resized() const { return resized; }
	bool is_visible() const { return (resolution.x > 0 && resolution.y > 0); }
	bool was_exited() const { return exited; }

private:
	const std::string TITLE;
	glm::ivec2 resolution;
	HINSTANCE instance;
	FILE *stream;
	HWND window;
	glm::ivec2 screen_resolution;
	bool resized;
	bool exited;

	static LRESULT CALLBACK window_callback(HWND window, UINT message,
		WPARAM message_information, LPARAM message_informaton_long);
};