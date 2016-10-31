#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor
	{
	public:
		Descriptor(){};
		~Descriptor(){ destroy(); };

		void create_layout(const Device *device, uint32_t binding, VkDescriptorType type, 
			VkShaderStageFlags stage_flags, uint32_t count = 1,
			VkSampler *immutable_samplers = nullptr);
		void allocate_set();
		void create_pool(uint32_t count = 1);
		void write(VkBuffer buffer, VkDeviceSize range, uint32_t binding,
			VkDescriptorType descriptor_type, uint32_t descriptor_count = 1,
			uint32_t element = 0, VkDeviceSize offset = 0);

		VkDescriptorSet get() const { return set; }
		VkDescriptorSetLayout get_layout() const { return layout; }
		VkDescriptorPool get_pool() const { return pool; }

	private:
		const Device *device;

		VkDescriptorType type;
		VkDescriptorSet set;
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;

		void destroy();
	};
}