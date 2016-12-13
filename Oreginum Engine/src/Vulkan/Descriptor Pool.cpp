#include "../Oreginum/Core.hpp"
#include "Descriptor Pool.hpp"

Oreginum::Vulkan::Descriptor_Pool::Descriptor_Pool(const Device& device,
	const std::vector<std::pair<vk::DescriptorType, uint32_t>>& sets) : device(&device)
{
	uint32_t descriptor_set_count{};
	std::vector<vk::DescriptorPoolSize> pool_sizes{sets.size()};
	for(uint32_t i{}; i < sets.size(); ++i)
	{
		descriptor_set_count += sets[i].second;
		pool_sizes[i] = {sets[i].first, sets[i].second};
	}

	vk::DescriptorPoolCreateInfo descriptor_pool_information{{}, descriptor_set_count,
		static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data()};

	if(device.get().createDescriptorPool(&descriptor_pool_information,
		nullptr, descriptor_pool.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan descriptor pool.");
}

Oreginum::Vulkan::Descriptor_Pool::~Descriptor_Pool()
{
	if(descriptor_pool.unique() && *descriptor_pool)
		device->get().destroyDescriptorPool(*descriptor_pool);
}

void Oreginum::Vulkan::Descriptor_Pool::swap(Descriptor_Pool *other)
{
	std::swap(this->device, other->device);
	std::swap(this->descriptor_pool, other->descriptor_pool);
}
