#pragma once
#include <string>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Shader
	{
	public:
		Shader(const Device& device, const std::vector<std::pair<
			std::string, vk::ShaderStageFlagBits>>& shaders);
		~Shader();

		const std::vector<vk::PipelineShaderStageCreateInfo>& get_information() const
		{ return information; }

	private:
		const Device& device;

		std::vector<vk::ShaderModule> modules;
		std::vector<vk::PipelineShaderStageCreateInfo> information;

		vk::ShaderModule create_shader_module(const std::string& shader);
	};
}