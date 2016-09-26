#include <array>
#include "Vulkan.hpp"

int WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	const std::vector<float> vertices
	{
		0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 1.0f, 0.0f, 0.0f
	};

	std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions{1};
	vertex_binding_descriptions[0].stride = sizeof(float)*5;
	vertex_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions{2};
	vertex_attribute_descriptions[0].binding = 0;
	vertex_attribute_descriptions[0].location = 0;
	vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	vertex_attribute_descriptions[0].offset = 0;

	vertex_attribute_descriptions[1].binding = 0;
	vertex_attribute_descriptions[1].location = 1;
	vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_attribute_descriptions[1].offset = sizeof(float)*2;

	Window window{"Oreginum Engine Vulkan Test", {1000, 600}, instance, true};
	Vulkan vulkan{window, "Oreginum Engine Vulkan Test", {0, 1, 0},
		"Oreginum Engine", {0, 1, 0}, {1, 0, 0}, vertex_binding_descriptions, 
		vertex_attribute_descriptions, vertices, true};

	while(!window.was_exited())
	{
		window.update();
		vulkan.render();
	}
}