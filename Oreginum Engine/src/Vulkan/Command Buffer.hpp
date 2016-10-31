#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"
#include "Command Pool.hpp"

namespace Oreginum::Vulkan
{
	class Command_Buffer
	{
	public:
		Command_Buffer(){}
		~Command_Buffer(){ destroy(); };

		void initialize(const Device *device, const Command_Pool *command_pool,
			VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VkCommandBuffer get() const { return command_buffer; }

	private:
		const Device *device;
		const Command_Pool *command_pool;

		void destroy();

		VkCommandBuffer command_buffer;
	};
}