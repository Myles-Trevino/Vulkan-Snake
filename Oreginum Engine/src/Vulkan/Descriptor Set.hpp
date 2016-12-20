#pragma once
#include "Device.hpp"
#include "Descriptor Pool.hpp"
#include "Buffer.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Set
	{
	public:
		Descriptor_Set(){}
		Descriptor_Set(const Device& device, const Descriptor_Pool& pool,
			vk::DescriptorType type, vk::ShaderStageFlags stage_flags,
			const Buffer& buffer = {}, uint32_t range = 0);
		Descriptor_Set *Descriptor_Set::operator=(Descriptor_Set other)
		{ swap(&other); return this; }
		~Descriptor_Set();

		void write(const Buffer& buffer, uint32_t range);

		const vk::DescriptorSet& get() const { return descriptor_set; }
		const vk::DescriptorSetLayout& get_layout() const { return *layout; }

	private:
		const Device *device;
		vk::DescriptorType type;
		vk::DescriptorSet descriptor_set;
		std::shared_ptr<vk::DescriptorSetLayout> layout =
			std::make_shared<vk::DescriptorSetLayout>();

		void swap(Descriptor_Set *other);
	};
}