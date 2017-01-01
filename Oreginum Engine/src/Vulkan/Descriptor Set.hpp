#pragma once
#include "Device.hpp"
#include "Descriptor Pool.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Set
	{
	public:
		struct Write_Information
		{
			vk::DescriptorType type;
			const vk::DescriptorBufferInfo *buffer;
			const vk::DescriptorImageInfo *image;
		};

		Descriptor_Set(){}
		Descriptor_Set(const Device& device, const Descriptor_Pool& pool,
			const std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>& bindings);
		Descriptor_Set *operator=(Descriptor_Set other){ swap(&other); return this; }
		~Descriptor_Set();

		void write(const std::vector<Write_Information>& write_descriptor_sets);

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