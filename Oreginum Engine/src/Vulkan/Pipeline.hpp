#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Shader.hpp"
#include "Descriptor Set.hpp"
#include "Render Pass.hpp"
#include "Swapchain.hpp"

namespace Oreginum::Vulkan
{
	class Pipeline
	{
	public:
		Pipeline(){}
		Pipeline(const Device& device, const Swapchain& swapchain,
			const Render_Pass& render_pass, const Shader& shader,
			const Vulkan::Descriptor_Set& descriptor_set, bool three_dimensional = false,
			bool model = false, const Pipeline& base = {});
		Pipeline *operator=(Pipeline other){ swap(&other); return this; };
		~Pipeline();

		const vk::Pipeline& get() const { return *pipeline; }
		const vk::PipelineLayout& get_layout() const { return pipeline_layout; }

	private:
		const Device *device;
		vk::PipelineLayout pipeline_layout;
		std::shared_ptr<vk::Pipeline> pipeline = std::make_shared<vk::Pipeline>();

		void swap(Pipeline *other);
	};
}