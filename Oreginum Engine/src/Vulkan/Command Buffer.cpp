#include "../Oreginum/Core.hpp"
#include "Command Buffer.hpp"

void Oreginum::Vulkan::Command_Buffer::destroy()
{
	if(command_buffer)
		vkFreeCommandBuffers(device->get(), command_pool->get(), 1, &command_buffer);
}

void Oreginum::Vulkan::Command_Buffer::initialize(const Device *device,
	const Command_Pool *command_pool, VkCommandBufferLevel level)
{
	this->device = device;
	this->command_pool = command_pool;
	destroy();

	VkCommandBufferAllocateInfo command_buffer_information;
	command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_information.pNext = nullptr;
	command_buffer_information.commandPool = command_pool->get();
	command_buffer_information.level = level;
	command_buffer_information.commandBufferCount = 1;

	if(vkAllocateCommandBuffers(device->get(), &command_buffer_information, &command_buffer) !=
		VK_SUCCESS) Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
}