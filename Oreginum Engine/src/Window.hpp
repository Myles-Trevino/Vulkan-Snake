#pragma once
#include <string>
#include <windows.h>
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Window
	{
		void initialize(const std::string& title,
			const glm::ivec2& resolution, bool debug = false);

		void update();

		HINSTANCE get_instance();
		HWND get();
		std::string get_title();
		glm::ivec2 get_resolution();
		bool was_closed();
		bool was_resized();
		bool is_visible();
	};
}