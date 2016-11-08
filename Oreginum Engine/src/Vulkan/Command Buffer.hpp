#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Command Pool.hpp"

namespace Oreginum::Vulkan
{
	class Command_Buffer
	{
	public:
		Command_Buffer(const Device& device, const Command_Pool& command_pool,
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		~Command_Buffer();

		const vk::CommandBuffer& get() const { return command_buffer; }

	private:
		const Device& device;
		const Command_Pool& command_pool;

		vk::CommandBuffer command_buffer;
	};
}