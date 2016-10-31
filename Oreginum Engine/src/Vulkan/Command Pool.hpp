#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Command_Pool
	{
	public:
		Command_Pool(){}
		~Command_Pool(){ destroy(); };

		void initialize(const Device *device, VkCommandPoolCreateFlags flags,
			uint32_t queue_family_index);

		VkCommandPool get() const { return command_pool; }

	private:
		const Device *device;

		void destroy();

		VkCommandPool command_pool;
	};
}