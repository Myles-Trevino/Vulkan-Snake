#pragma once
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