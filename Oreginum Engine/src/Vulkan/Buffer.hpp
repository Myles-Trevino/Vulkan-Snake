#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Command Pool.hpp"

namespace Oreginum::Vulkan
{
	class Buffer
	{
	public:
		Buffer(const Device& device, const Command_Pool& command_pool,
			vk::BufferUsageFlags flags, const void *data = nullptr, size_t size = 0);
		~Buffer();

		void write(const void *data = nullptr, size_t size = 0);

		const vk::Buffer& get() const { return buffer; }

	private:
		const Device& device;
		const Command_Pool& command_pool;

		vk::Buffer buffer;
		vk::DeviceMemory buffer_memory;
		vk::Buffer stage;
		vk::DeviceMemory stage_memory;

		void create_buffer(vk::Buffer *buffer, vk::DeviceMemory *memory, size_t size,
			vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_property_flags);
	};
}