#include <Windows.h>
#include "Core.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

namespace
{
	glm::ivec2 position;
	glm::ivec2 delta;
	bool visible;

	void center(){ SetCursorPos(Oreginum::Core::get_screen_resolution().x/2,
		Oreginum::Core::get_screen_resolution().y/2); }
}

void Oreginum::Mouse::initialize()
{
	ShowCursor(false);
	center();
}

void Oreginum::Mouse::update()
{
	if(Keyboard::was_pressed(Key::ESC))
	{
		if(visible) visible = false, ShowCursor(false), center();
		else visible = true, ShowCursor(true);
	}
	if(visible) return;
	static POINT point{};
	GetCursorPos(&point);
	position = {point.x, point.y};
	delta = position-Core::get_screen_resolution()/2;
	center();
}

glm::ivec2 Oreginum::Mouse::get_delta(){ return delta; }