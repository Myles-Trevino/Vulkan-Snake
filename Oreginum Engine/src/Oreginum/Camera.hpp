#pragma once
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Camera
	{
		void update();

		glm::fvec3 get_position();
		glm::fmat4 get_view();
		glm::fmat4 get_projection();
	}
}