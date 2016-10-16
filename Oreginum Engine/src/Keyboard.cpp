#include "Keyboard.hpp"

namespace
{
	bool pressed[255];
	bool held[255];
}

void Oreginum::Keyboard::update()
{
	for(bool& k : pressed) k = false;

	static MSG message;
	while(PeekMessage(&message, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
		if(message.message == WM_KEYDOWN)
			pressed[message.wParam] = true, held[message.wParam] = true;
		else if(message.message == WM_KEYUP) held[message.wParam] = false;
}

bool Oreginum::Keyboard::was_pressed(Key key){ return pressed[key]; }

bool Oreginum::Keyboard::is_held(Key key){ return held[key]; }