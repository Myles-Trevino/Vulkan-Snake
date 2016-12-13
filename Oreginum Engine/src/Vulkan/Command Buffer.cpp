#include "../Oreginum/Core.hpp"
#include "Command Buffer.hpp"

Oreginum::Vulkan::Command_Buffer::Command_Buffer(const Device& device,
	const Command_Pool& command_pool, vk::CommandBufferLevel level)
	: device(&device), command_pool(&command_pool)
{
	vk::CommandBufferAllocateInfo command_buffer_information{command_pool.get(), level, 1};
	if(device.get().allocateCommandBuffers(&command_buffer_information,
		&command_buffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
}

void Oreginum::Vulkan::Command_Buffer::swap(Command_Buffer *other)
{
	std::swap(this->device, other->device);
	std::swap(this->command_pool, other->command_pool);
	std::swap(this->command_buffer, other->command_buffer);
}

vk::CommandBuffer Oreginum::Vulkan::Command_Buffer::begin_single_time_commands(
	const Device& device, const Command_Pool& temporary_command_pool)
{
	vk::CommandBufferAllocateInfo command_buffer_allocate_information
	{temporary_command_pool.get(), vk::CommandBufferLevel::ePrimary, 1};
	vk::CommandBuffer temporary_command_buffer;
	device.get().allocateCommandBuffers(&command_buffer_allocate_information,
		&temporary_command_buffer);

	vk::CommandBufferBeginInfo command_buffer_begin_information
	{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr};
	temporary_command_buffer.begin(command_buffer_begin_information);

	return temporary_command_buffer;
}

void Oreginum::Vulkan::Command_Buffer::end_single_time_commands(
	const Device& device, const Command_Pool& temporary_command_pool,
	const vk::CommandBuffer& temporary_command_buffer)
{
	temporary_command_buffer.end();
	vk::SubmitInfo submit_information
	{0, nullptr, nullptr, 1, &temporary_command_buffer, 0, nullptr};
	device.get_graphics_queue().submit(submit_information, VK_NULL_HANDLE);
	device.get_graphics_queue().waitIdle();
	device.get().freeCommandBuffers(temporary_command_pool.get(), temporary_command_buffer);
}