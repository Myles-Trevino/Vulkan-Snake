#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Buffer.hpp"

Oreginum::Vulkan::Buffer::Buffer(const Device& device, const Command_Pool& command_pool,
	vk::BufferUsageFlags flags, const void *data, size_t size)
	: device(device), command_pool(command_pool)
{
	if(!size) return;

	create_buffer(&stage, &stage_memory, size, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	create_buffer(&buffer, &buffer_memory, size, flags |
		vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

	if(data) write(data, size);
}

Oreginum::Vulkan::Buffer::~Buffer()
{
	if(buffer) vkDestroyBuffer(device.get(), buffer, nullptr);
	if(buffer_memory) vkFreeMemory(device.get(), buffer_memory, nullptr);
	if(stage) vkDestroyBuffer(device.get(), stage, nullptr);
	if(stage_memory) vkFreeMemory(device.get(), stage_memory, nullptr);
}

void Oreginum::Vulkan::Buffer::create_buffer(vk::Buffer *buffer, vk::DeviceMemory *memory,
	size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_property_flags)
{
	//Create buffer
	vk::BufferCreateInfo buffer_information{{}, size, usage};

	if(device.get().createBuffer(&buffer_information, nullptr, buffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan buffer.");

	//Allocate buffer memory
	vk::MemoryRequirements memory_requirements
	(device.get().getBufferMemoryRequirements(*buffer));

	vk::MemoryAllocateInfo memory_information{memory_requirements.size,
		Core::find_memory(device, memory_requirements.memoryTypeBits, memory_property_flags)};

	if(device.get().allocateMemory(&memory_information, nullptr, memory) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate memory for a Vulkan buffer.");

	//Bind buffer memory
	if(device.get().bindBufferMemory(*buffer, *memory, 0) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not bind memory to a Vulkan buffer.");
}

void Oreginum::Vulkan::Buffer::write(const void *data, size_t size)
{
	//Copy data to stage buffer
	auto result{device.get().mapMemory(stage_memory, 0, size)};
	if(result.result != vk::Result::eSuccess)
		Oreginum::Core::error("Could not map Vulkan staging buffer memory.");

	memcpy(result.value, data, size);

	device.get().unmapMemory(stage_memory);

	//Copy data from stage buffer to device buffer
	vk::CommandBuffer command_buffer{Core::begin_single_time_commands(device, command_pool)};
	command_buffer.copyBuffer(stage, buffer, {0, 0, size});
	Core::end_single_time_commands(device, command_pool, command_buffer);
}