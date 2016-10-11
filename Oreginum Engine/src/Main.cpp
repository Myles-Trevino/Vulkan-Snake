#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "Vulkan.hpp"
#include "Core.hpp"

/*
	-Textures aren't loaded optimally
	-Textures have to be power of two
*/

int WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	//Model data
	const std::vector<float> vertices
	{
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	const std::vector<uint16_t> indices{0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8};

	std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions{1};
	vertex_binding_descriptions[0].stride = sizeof(float)*8;
	vertex_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions{3};
	vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_attribute_descriptions[1].location = 1;

	vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_attribute_descriptions[1].offset = sizeof(float)*3;

	vertex_attribute_descriptions[2].location = 2;
	vertex_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertex_attribute_descriptions[2].offset = sizeof(float)*6;

	struct Uniforms{ glm::fmat4 mvp; } uniforms;

	//Initialize
	Oreginum::Window window{"Oreginum Engine Vulkan Test", {1000, 600}, instance};
	Oreginum::Vulkan vulkan{window, "Oreginum Engine Vulkan Test", {0, 1, 0},
		"Oreginum Engine", {0, 1, 0}, {1, 0, 26}, vertex_binding_descriptions, 
		vertex_attribute_descriptions, vertices, indices, &uniforms, sizeof(uniforms),
		"Resources/Textures/Garden.png"};

	//Program
	double previous_time{Oreginum::Core::time()}, delta{};
	const double MINIMUM_DELTA{1.0/Oreginum::Core::refresh_rate()};

	while(window.update())
	{
		//Update MVP matrix
		uniforms.mvp = glm::perspective(glm::radians(45.0f), vulkan.get_swapchain_extent().x/
			(float)vulkan.get_swapchain_extent().y, 0.1f, 10.0f);
		uniforms.mvp[1][1] *= -1;
		uniforms.mvp *= glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f))*glm::rotate(glm::mat4(), (float)Oreginum::Core::time()*
				glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		vulkan.update_uniform_buffer();

		//Render
		vulkan.render();

		//Limit framerate and sleep thread
		while((delta = Oreginum::Core::time()-previous_time) < MINIMUM_DELTA)
			if(MINIMUM_DELTA-delta < 0.0015) Sleep(0); else Sleep(1);
		previous_time = Oreginum::Core::time();
	}
}