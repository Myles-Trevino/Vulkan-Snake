#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Shader.hpp"
#include "Descriptor Set.hpp"
#include "Render Pass.hpp"

namespace Oreginum::Vulkan
{
	class Pipeline
	{
	public:
		Pipeline(const Device& device, const Shader& shader,
			const Descriptor_Set& descriptor_set, const Render_Pass& render_pass);
		~Pipeline();

		const vk::Pipeline& get() const { return pipeline; }
		const vk::PipelineLayout& get_layout() const { return pipeline_layout; }

	private:
		const Device& device;

		vk::PipelineLayout pipeline_layout;
		vk::Pipeline pipeline;
	};
}