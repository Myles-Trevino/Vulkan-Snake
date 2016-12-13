#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "../Oreginum/Core.hpp"
#include "Buffer.hpp"

Oreginum::Vulkan::Buffer::Buffer(const Device& device,
	const Command_Pool& temporary_command_pool, vk::BufferUsageFlags usage,
	size_t size, const void *data) : device(&device),
	temporary_command_pool(&temporary_command_pool), size(size)
{
	create_buffer(&stage, &stage_memory, size, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	create_buffer(buffer.get(), &buffer_memory, size, usage |
		vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
	if(data) write(data, size);
}

Oreginum::Vulkan::Buffer::~Buffer()
{
	if(!buffer.unique()) return;
	if(*buffer) vkDestroyBuffer(device->get(), *buffer, nullptr);
	if(buffer_memory) vkFreeMemory(device->get(), buffer_memory, nullptr);
	if(stage) vkDestroyBuffer(device->get(), stage, nullptr);
	if(stage_memory) vkFreeMemory(device->get(), stage_memory, nullptr);
}

void Oreginum::Vulkan::Buffer::swap(Buffer *other)
{
	std::swap(this->device, other->device);
	std::swap(this->temporary_command_pool, other->temporary_command_pool);
	std::swap(this->size, other->size);
	std::swap(this->buffer, other->buffer);
	std::swap(this->buffer_memory, other->buffer_memory);
	std::swap(this->stage, other->stage);
	std::swap(this->stage_memory, other->stage_memory);
}

void Oreginum::Vulkan::Buffer::create_buffer(vk::Buffer *buffer, vk::DeviceMemory *memory,
	size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_property_flags)
{
	//Create buffer
	vk::BufferCreateInfo buffer_information{{}, size, usage};
	if(device->get().createBuffer(&buffer_information, nullptr, buffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan buffer.");

	//Allocate buffer memory
	vk::MemoryRequirements memory_requirements
	(device->get().getBufferMemoryRequirements(*buffer));
	vk::MemoryAllocateInfo memory_information{memory_requirements.size,
		find_memory(*device, memory_requirements.memoryTypeBits, memory_property_flags)};
	if(device->get().allocateMemory(&memory_information,
		nullptr, memory) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate memory for a Vulkan buffer.");

	//Bind buffer memory
	if(device->get().bindBufferMemory(*buffer, *memory, 0) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not bind memory to a Vulkan buffer.");
}

uint32_t Oreginum::Vulkan::Buffer::find_memory(const Device& device,
	uint32_t type, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memory_properties(device.get_gpu().getMemoryProperties());
	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags &
			properties) == properties) return i;
	Oreginum::Core::error("Could not find suitable Vulkan memory.");
}

void Oreginum::Vulkan::Buffer::write(const void *data, size_t size)
{
	//Copy data to stage buffer
	this->size = size;
	auto result{device->get().mapMemory(stage_memory, 0, size)};
	if(result.result != vk::Result::eSuccess)
		Oreginum::Core::error("Could not map Vulkan staging buffer memory.");
	memcpy(result.value, data, size);
	device->get().unmapMemory(stage_memory);

	//Copy data from stage buffer to device buffer
	vk::CommandBuffer temporary_command_buffer
	{Command_Buffer::begin_single_time_commands(*device, *temporary_command_pool)};
	temporary_command_buffer.copyBuffer(stage, *buffer, {{0, 0, size}});
	Command_Buffer::end_single_time_commands(*device,
		*temporary_command_pool, temporary_command_buffer);
}