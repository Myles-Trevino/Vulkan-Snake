#include "Window.hpp"
#include "Vulkan.hpp"

int main()
{
	Window window{"Oreginum Engine Vulkan Test", {1000, 600}};
	Vulkan vulkan{window, "Oreginum Engine Vulkan Test", {0, 1, 0},
		"Oreginum Engine", {0, 1, 0}, {1, 0, 0}, true};

	while(!window.was_closed())
	{
		window.update();
		vulkan.render();
	}
}