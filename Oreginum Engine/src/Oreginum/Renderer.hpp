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
		const Vulkan::Descriptor_Pool& get_descriptor_pool();
		const Vulkan::Buffer& get_uniform_buffer();
		uint32_t get_padded_uniform_size();
		const Vulkan::Pipeline& get_primitive_2d_pipeline();
		const Vulkan::Pipeline& get_sprite_pipeline();
		const Vulkan::Pipeline& get_primitive_3d_pipeline();
		const Vulkan::Pipeline& get_model_pipeline();
		const Vulkan::Pipeline& get_environment_pipeline();
	};
}