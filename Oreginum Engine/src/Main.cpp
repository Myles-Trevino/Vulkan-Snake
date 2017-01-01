#include "Oreginum/Core.hpp"
#include "Oreginum/Keyboard.hpp"
#include "Oreginum/Camera.hpp"
#include "Oreginum/Renderer.hpp"
#include "Oreginum/Rectangle.hpp"
#include "Oreginum/Sprite.hpp"
#include "Oreginum/Cuboid.hpp"
#include "Oreginum/Model.hpp"
#include "Oreginum/Environment.hpp"

/*
	See about using multiple shaders per pipeline.
	Make sure there are no Renderer functions called in Vulkan classes.
*/

int WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	Oreginum::Core::initialize("Oreginum Engine Vulkan Test", {666, 666}, true);

	Oreginum::Environment environment{"Resources/Textures/Fila Night"};
	Oreginum::Model model{"Resources/Models/Suzanne/Suzanne.dae", {0, 0, 3}};
	Oreginum::Cuboid light{{3, -3, 0}, {.3f, .3f, .3f}, {1, 0, 0}},
		light_2{{-3, -3, 0}, {.3f, .3f, .3f}, {0, 1, 0}};
	Oreginum::Renderer::add({&environment, &model, &light, &light_2});

	while(Oreginum::Core::update())
	{
		model.rotate(Oreginum::Core::get_delta()*.3f, {0, 1, 0});
	}

	Oreginum::Core::destroy();
}