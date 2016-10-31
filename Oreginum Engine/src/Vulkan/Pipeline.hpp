#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"
#include "Shader.hpp"
#include "Descriptor.hpp"
#include "Render Pass.hpp"

namespace Oreginum::Vulkan
{
	class Pipeline
	{
	public:
		Pipeline(){};
		~Pipeline(){ destroy(); }

		void initialize(const Device *device, const Shader& shader,
			const Descriptor& descriptor, const Render_Pass& render_pass);

		VkPipeline get() const { return pipeline; }
		VkPipelineLayout get_layout() const { return pipeline_layout; }

	private:
		const Device *device;

		VkPipelineLayout pipeline_layout;
		VkPipeline pipeline;

		void destroy();
	};
}