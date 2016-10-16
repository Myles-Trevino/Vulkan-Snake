#include <windows.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtc/matrix_transform.hpp>
#include "Core.hpp"
#include "Vulkan.hpp"
#include "Camera.hpp"

int WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	Oreginum::Model model{"Resources/Models/Suzanne/Suzanne.dae"};
	struct Uniforms{ glm::fmat4 model, view, projection; glm::fvec3 camera_position; } uniforms;
	Oreginum::Core::initialize("Oreginum Engine Vulkan Test",
		{900, 900}, &model, &uniforms, sizeof(uniforms), true);

	while(Oreginum::Core::update())
	{
		uniforms.model = glm::translate(glm::mat4{}, glm::fvec3{0, 0, 1.3f})*
			glm::rotate(glm::mat4{}, Oreginum::Core::get_time(), {0, 1, 0})*
			glm::rotate(glm::mat4{}, glm::radians(90.0f), {1, 0, 0});
		uniforms.view = Oreginum::Camera::get_view();
		uniforms.projection = Oreginum::Camera::get_projection();
		uniforms.projection[1][1] *= -1;
		uniforms.camera_position = Oreginum::Camera::get_position();

		Oreginum::Vulkan::render();
	}
	Oreginum::Core::destroy();
}