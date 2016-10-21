#pragma once
#include <GLM\glm.hpp>
#include "Model.hpp"

namespace Oreginum
{
	namespace Vulkan
	{
		void initialize(const void *vertex_data, size_t vertex_data_size,
			const void *uniform_buffer_object, size_t uniform_buffer_object_size,
			bool debug = false);
		void destroy();

		void render();
	};
}