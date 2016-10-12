#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "Vulkan.hpp"
#include "Core.hpp"

int WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	//Initialize
	Oreginum::Window window{"Oreginum Engine Vulkan Test", {600, 600}, instance, true};
	Oreginum::Model model{"Resources/Models/Suzanne/Suzanne.dae"};
	struct Uniforms{ glm::fmat4 mvp; } uniforms;
	Oreginum::Vulkan vulkan{window, "Oreginum Engine Vulkan Test", {0, 1, 0}, "Oreginum Engine",
		{0, 1, 0}, {1, 0, 26}, model, &uniforms, sizeof(uniforms), true};

	//Program
	double previous_time{Oreginum::Core::time()}, delta{};
	const double MINIMUM_DELTA{1.0/Oreginum::Core::refresh_rate()};
	while(window.update())
	{
		//Update MVP matrix
		uniforms.mvp = glm::perspective(glm::radians(45.0f), vulkan.get_swapchain_extent().x/
			(float)vulkan.get_swapchain_extent().y, 0.1f, 10.0f);
		uniforms.mvp[1][1] *= -1;
		uniforms.mvp *= glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f))*glm::rotate(glm::mat4(), (float)Oreginum::Core::time()*
				glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		vulkan.update_uniform_buffer();

		//Render
		vulkan.render();

		//Limit framerate and sleep thread
		timeBeginPeriod(1);
		while((delta = Oreginum::Core::time()-previous_time) < MINIMUM_DELTA)
			if(MINIMUM_DELTA-delta < 0.003) Sleep(0); else Sleep(1);
		previous_time = Oreginum::Core::time();
		timeEndPeriod(1);
	}
}