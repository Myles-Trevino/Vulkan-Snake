#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Buffer.hpp"

void Oreginum::Vulkan::Buffer::initialize(const Device *device,
	const void *data, size_t size, VkBufferUsageFlags flags)
{
	this->device = device;
	if(!size) return;
	destroy();
	create_buffer(&stage, &stage_memory, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	create_buffer(&buffer, &buffer_memory, size, flags |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(!data) return;
	fill(data, size);
}

void Oreginum::Vulkan::Buffer::destroy()
{
	if(buffer) vkDestroyBuffer(device->get(), buffer, nullptr);
	if(buffer_memory) vkFreeMemory(device->get(), buffer_memory, nullptr);
	if(stage) vkDestroyBuffer(device->get(), stage, nullptr);
	if(stage_memory) vkFreeMemory(device->get(), stage_memory, nullptr);
}

void Oreginum::Vulkan::Buffer::create_buffer(VkBuffer *buffer, VkDeviceMemory *memory,
	size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_property_flags,
	VkSharingMode sharing_mode, VkDeviceSize offset, uint32_t queue_family_index_count,
	const uint32_t *queue_family_indices)
{
	//Create buffer
	VkBufferCreateInfo buffer_information;
	buffer_information.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_information.pNext = nullptr;
	buffer_information.flags = NULL;
	buffer_information.size = size;
	buffer_information.usage = usage;
	buffer_information.sharingMode = sharing_mode;
	buffer_information.queueFamilyIndexCount = queue_family_index_count;
	buffer_information.pQueueFamilyIndices = queue_family_indices;

	if(vkCreateBuffer(device->get(), &buffer_information, nullptr, buffer) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan buffer.");

	//Allocate buffer memory
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device->get(), *buffer, &memory_requirements);

	VkMemoryAllocateInfo memory_information;
	memory_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_information.pNext = nullptr;
	memory_information.allocationSize = memory_requirements.size;
	memory_information.memoryTypeIndex =
		Core::find_memory(memory_requirements.memoryTypeBits, memory_property_flags);

	if(vkAllocateMemory(device->get(), &memory_information, nullptr, memory) != VK_SUCCESS)
		Oreginum::Core::error("Could not allocate memory for a Vulkan buffer.");

	//Bind buffer memory
	if(vkBindBufferMemory(device->get(), *buffer, *memory, offset) != VK_SUCCESS)
		Oreginum::Core::error("Could not bind memory to a Vulkan buffer.");
}

void Oreginum::Vulkan::Buffer::fill(const void *data, size_t size)
{
	//Copy data to staging buffer
	void *memory_pointer;
	if(vkMapMemory(device->get(), stage_memory, 0, size, NULL, &memory_pointer))
		Oreginum::Core::error("Could not map Vulkan staging buffer memory.");

	memcpy(memory_pointer, data, size);

	VkMappedMemoryRange mapped_memory_range;
	mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped_memory_range.pNext = nullptr;
	mapped_memory_range.memory = stage_memory;
	mapped_memory_range.offset = 0;
	mapped_memory_range.size = size;

	vkFlushMappedMemoryRanges(device->get(), 1, &mapped_memory_range);
	vkUnmapMemory(device->get(), stage_memory);

	//Copy data to device buffer
	VkCommandBuffer command_buffer{Core::begin_single_time_commands()};
	VkBufferCopy buffer_copy{0, 0, size};
	vkCmdCopyBuffer(command_buffer, stage, buffer, 1, &buffer_copy);
	Core::end_single_time_commands(command_buffer);
}