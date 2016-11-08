#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Command_Pool
	{
	public:
		Command_Pool(const Device& device, uint32_t queue_family_index,
			vk::CommandPoolCreateFlags flags = {});
		~Command_Pool();

		const vk::CommandPool& get() const { return command_pool; }

	private:
		const Device& device;

		vk::CommandPool command_pool;
	};
}