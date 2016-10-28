#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Buffer
	{
	public:
		Buffer(){}
		~Buffer();

		void initialize(const Device *device, const void *data = nullptr, size_t size = 0,
			VkBufferUsageFlags flags = NULL);
		void fill(const void *data = nullptr, size_t size = 0);

		const VkBuffer *get() const { return &buffer; }

	private:
		const Device *device;
		VkBuffer buffer;
		VkDeviceMemory buffer_memory;
		VkBuffer stage;
		VkDeviceMemory stage_memory;

		void create_buffer(VkBuffer *buffer, VkDeviceMemory *memory, size_t size,
			VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_property_flags,
			VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
			VkDeviceSize offset = 0, uint32_t queue_family_index_count = 0,
			const uint32_t *queue_family_indices = nullptr);
	};
}