#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Pool
	{
	public:
		Descriptor_Pool(){}
		Descriptor_Pool(const Device& device,
			const std::vector<std::pair<vk::DescriptorType, uint32_t>>& sets);
		Descriptor_Pool *Descriptor_Pool::operator=(Descriptor_Pool other)
		{ swap(&other); return this; }
		~Descriptor_Pool();

		const vk::DescriptorPool& get() const { return *descriptor_pool; }

	private:
		const Device *device;
		std::shared_ptr<vk::DescriptorPool> descriptor_pool = 
			std::make_shared<vk::DescriptorPool>();

		void swap(Descriptor_Pool *other);
	};
}