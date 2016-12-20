#define NOMINMAX
#include <windows.h>
#include "Core.hpp"
#include "Window.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

namespace
{
	glm::ivec2 position;
	glm::ivec2 delta;
	bool locked;
	bool pressed[3];

	glm::ivec2 get_window_center()
	{ return Oreginum::Window::get_position()+Oreginum::Window::get_resolution()/2; }

	void center(){ SetCursorPos(get_window_center().x, get_window_center().y); }

	void lock(){ ShowCursor(false), center(), locked = true; }

	void free(){ ShowCursor(true), locked = false; }
}

void Oreginum::Mouse::initialize(){ lock(); }

void Oreginum::Mouse::destroy(){ free(); }

void Oreginum::Mouse::update()
{
	for(bool& b : pressed) b = false;

	if(Keyboard::was_pressed(Key::ESC)) if(locked) free(); else lock();

	static POINT point{};
	GetCursorPos(&point);
	if(locked)
	{
		position = {point.x, point.y};
		delta = position-get_window_center();
		center();
	}
	else
	{
		glm::ivec2 previous_position{position};
		position = {point.x, point.y};
		delta = position-previous_position;
	}
}

void Oreginum::Mouse::set_pressed(Button button, bool pressed){ ::pressed[button] = pressed; }

glm::ivec2 Oreginum::Mouse::get_position()
{
	POINT position{};
	GetCursorPos(&position);
	return glm::ivec2{position.x, position.y}-Window::get_position();
}

const glm::ivec2& Oreginum::Mouse::get_delta(){ return delta; }

bool Oreginum::Mouse::is_locked(){ return locked; }

bool Oreginum::Mouse::was_pressed(Button button){ return pressed[button]; }

bool Oreginum::Mouse::is_held(Button button)
{ return (GetAsyncKeyState(button) != 0) && Window::has_focus(); }