#include "../Oreginum/Core.hpp"
#include "Command Buffer.hpp"

Oreginum::Vulkan::Command_Buffer::Command_Buffer(const Device& device,
	const Command_Pool& command_pool, vk::CommandBufferLevel level)
	: device(&device), command_pool(&command_pool)
{
	vk::CommandBufferAllocateInfo command_buffer_information{command_pool.get(), level, 1};
	if(device.get().allocateCommandBuffers(&command_buffer_information,
		command_buffer.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
}

Oreginum::Vulkan::Command_Buffer::~Command_Buffer()
{
	if(command_buffer.unique() && *command_buffer)
		device->get().freeCommandBuffers(command_pool->get(), {*command_buffer});
}

void Oreginum::Vulkan::Command_Buffer::swap(Command_Buffer *other)
{
	std::swap(device, other->device);
	std::swap(command_pool, other->command_pool);
	std::swap(command_buffer, other->command_buffer);
}

void Oreginum::Vulkan::Command_Buffer::begin(vk::CommandBufferUsageFlagBits flags) const
{
	vk::CommandBufferBeginInfo command_buffer_begin_information{flags};
	command_buffer->begin(command_buffer_begin_information);
}
void Oreginum::Vulkan::Command_Buffer::end() const
{
	if(command_buffer->end() != vk::Result::eSuccess)
		Core::error("Could not record a Vulkan command buffer.");
}

void Oreginum::Vulkan::Command_Buffer::submit() const
{
	vk::SubmitInfo submit_information{0, nullptr,
		nullptr, 1, command_buffer.get(), 0, nullptr};
	device->get_graphics_queue().submit(submit_information, VK_NULL_HANDLE);
	device->get_graphics_queue().waitIdle();
}