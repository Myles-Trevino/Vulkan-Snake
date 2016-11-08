#pragma once
#include <string>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include "Model.hpp"

namespace Oreginum
{
	namespace Core
	{
		void initialize(const std::string& title,
			const glm::ivec2& resolution, bool debug = false);
		void destroy();

		void error(const std::string& error);
		bool update();

		uint32_t get_refresh_rate();
		const glm::ivec2& get_screen_resolution();
		float get_time();
		float get_delta();
	}
}
