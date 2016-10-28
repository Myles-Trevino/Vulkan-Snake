#pragma once
#include <string>
#define NOMINMAX
#include <windows.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Window
	{
		void initialize(const std::string& title,
			const glm::ivec2& resolution, bool debug = false);
		void destroy();

		void update();

		HINSTANCE get_instance();
		HWND get();
		std::string get_title();
		glm::ivec2 get_resolution();
		glm::ivec2 get_position();
		bool was_closed();
		bool was_resized();
		bool is_visible();
		bool has_focus();
	};
}