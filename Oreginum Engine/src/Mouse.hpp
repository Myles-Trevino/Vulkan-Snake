#pragma once
#include <GLM/glm.hpp>

namespace Oreginum
{
	enum Button{LEFT_MOUSE = VK_LBUTTON, RIGHT_MOUSE = VK_RBUTTON, MIDDLE_MOUSE = VK_MBUTTON};

	namespace Mouse
	{
		void initialize();
		void destroy();

		void update();
		void set_pressed(Button button, bool pressed = true);

		glm::ivec2 get_delta();
		bool is_locked();
		bool was_pressed(Button button);
		bool is_held(Button button);
	}
}