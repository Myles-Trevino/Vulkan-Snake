#pragma once
#include "Renderable.hpp"
#include "../Vulkan/Command Pool.hpp"

namespace Oreginum
{
	namespace Renderer
	{
		void initialize(bool debug);
		void add(Renderable *renderable);
		void add(const std::vector<Renderable *>& renderables);
		void remove(uint32_t index);
		void clear();
		void record();
		void reinitialize_swapchain();
		void render();

		const Vulkan::Device& get_device();
		const Vulkan::Command_Buffer& get_temporary_command_buffer();
	};
}