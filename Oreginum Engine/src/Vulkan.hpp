#pragma once
#include "Model.hpp"

namespace Oreginum
{
	namespace Vulkan
	{
		void initialize(const Model *model, const void *uniform_buffer_object,
			const size_t uniform_buffer_object_size, bool debug = false);
		void destroy();

		void render();
	};
}