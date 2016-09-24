#include "Vulkan.hpp"

int WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	Window window{"Oreginum Engine Vulkan Test", {1000, 600}, instance, true};

	Vulkan vulkan{window, "Oreginum Engine Vulkan Test", {0, 1, 0},
		"Oreginum Engine", {0, 1, 0}, {1, 0, 0}, true};

	while(!window.was_exited())
	{
		window.update();
		vulkan.render();
	}
}
