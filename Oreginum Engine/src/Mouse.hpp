#pragma once
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Mouse
	{
		void initialize();
		void update();
		glm::ivec2 get_delta();
	}
}