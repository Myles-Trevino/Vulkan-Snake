#include "../Oreginum/Core.hpp"
#include "Command Buffer.hpp"

Oreginum::Vulkan::Command_Buffer::Command_Buffer(const Device& device,
	const Command_Pool& command_pool, vk::CommandBufferLevel level)
	: device(device), command_pool(command_pool)
{
	vk::CommandBufferAllocateInfo command_buffer_information{command_pool.get(), level, 1};

	if(device.get().allocateCommandBuffers(&command_buffer_information,
		&command_buffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
}

Oreginum::Vulkan::Command_Buffer::~Command_Buffer()
{ device.get().freeCommandBuffers(command_pool.get(), command_buffer); }