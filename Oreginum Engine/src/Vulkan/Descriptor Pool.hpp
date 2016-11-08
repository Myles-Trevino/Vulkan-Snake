#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Pool
	{
	public:
		Descriptor_Pool(const Device& device,
			const std::vector<std::pair<vk::DescriptorType, uint32_t>>& sets);
		~Descriptor_Pool();

		const vk::DescriptorPool& get() const { return descriptor_pool; }

	private:
		const Device& device;

		vk::DescriptorPool descriptor_pool;
	};
}