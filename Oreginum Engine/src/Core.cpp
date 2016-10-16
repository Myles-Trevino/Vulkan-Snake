#include <chrono>
#include <windows.h>
#include "Vulkan.hpp"
#include "Window.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Camera.hpp"
#include "Core.hpp"

namespace
{
	glm::ivec2 screen_resolution;
	int refresh_rate;
	float previous_time, delta;
	float minimum_delta;
	static double initial_time;

	double time_since_epoch(){ return std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::high_resolution_clock::now().time_since_epoch()).count()/1000000.0; }
};

void Oreginum::Core::initialize(const std::string& title, const glm::ivec2& resolution,
	const Model *model, const void *uniform_buffer_object,
	size_t uniform_buffer_object_size, bool debug)
{
	screen_resolution = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	DEVMODE devmode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	refresh_rate = devmode.dmDisplayFrequency;
	minimum_delta = 1.f/Oreginum::Core::get_refresh_rate();
	initial_time = time_since_epoch();

	Window::initialize(title, resolution, debug);
	Vulkan::initialize(model, uniform_buffer_object, uniform_buffer_object_size, debug);
	Mouse::initialize();
}

void Oreginum::Core::destroy()
{
	Vulkan::destroy();
}

void Oreginum::Core::error(const std::string& error)
{
	destroy();
	MessageBox(NULL, error.c_str(), "Oreginum Engine", MB_ICONERROR);
	std::exit(EXIT_FAILURE);
}

bool Oreginum::Core::update()
{
	timeBeginPeriod(1);
	while((delta = get_time()-previous_time) < minimum_delta)
		if(minimum_delta-delta < 0.003f) Sleep(0); else Sleep(1);
	previous_time = get_time();
	timeEndPeriod(1);

	Window::update();
	Keyboard::update();
	Mouse::update();
	Camera::update();
	return !Window::was_closed();
}

int Oreginum::Core::get_refresh_rate(){ return refresh_rate; }

glm::ivec2 Oreginum::Core::get_screen_resolution(){ return screen_resolution; }

float Oreginum::Core::get_time(){ return static_cast<float>(time_since_epoch()-initial_time); }

float Oreginum::Core::get_delta(){ return delta; }