#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Shader
	{
	public:
		Shader(const Device *device) : device(device){};
		~Shader();

		void add(const std::string& shader, VkShaderStageFlagBits stage);

		const std::vector<VkPipelineShaderStageCreateInfo>& get_information() const
		{ return information; }

	private:
		const Device *device;

		std::vector<VkShaderModule> modules;
		std::vector<VkPipelineShaderStageCreateInfo> information;

		VkShaderModule create_shader_module(const std::string& shader);
	};
}