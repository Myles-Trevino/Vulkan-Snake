#pragma once
#include "Device.hpp"
#include "Descriptor Pool.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Set
	{
	public:
		Descriptor_Set(const Device& device, const Descriptor_Pool& pool,
			vk::DescriptorType type, vk::ShaderStageFlags stage_flags);
		~Descriptor_Set();

		void write(vk::Buffer buffer);

		const vk::DescriptorSet& get() const { return descriptor_set; }
		const vk::DescriptorSetLayout& get_layout() const { return layout; }

	private:
		const Device& device;
		vk::DescriptorType type;

		vk::DescriptorSet descriptor_set;
		vk::DescriptorSetLayout layout;
	};
}