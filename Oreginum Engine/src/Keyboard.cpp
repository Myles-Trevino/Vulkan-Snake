#include "Window.hpp"
#include "Keyboard.hpp"

namespace{ bool pressed[255]; }

void Oreginum::Keyboard::update(){ for(bool& k : pressed) k = false; }

void Oreginum::Keyboard::set_pressed(Key key, bool pressed){ ::pressed[key] = pressed; }

bool Oreginum::Keyboard::was_pressed(Key key){ return pressed[key]; }

bool Oreginum::Keyboard::is_held(Key key)
{ return (GetAsyncKeyState(key) != 0) && Window::has_focus(); }